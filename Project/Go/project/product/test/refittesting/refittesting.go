// Copyright 2009 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package refittesting

import (
	"bytes"
	"dave/product/test/refittesting/race"
	"errors"
	"flag"
	"fmt"
	"io"
	"math/rand"
	"os"
	"reflect"
	"regexp"
	"runtime"
	"runtime/debug"
	"strconv"
	"strings"
	"sync"
	"sync/atomic"
	"time"
)

var initRan bool

// Init registers testing flags. These flags are automatically registered by
// the "go test" command before running test functions, so Init is only needed
// when calling functions such as Benchmark without using "go test".
//
// Init has no effect if it was already called.
func Init() {
	if initRan {
		return
	}
	initRan = true
	// The short flag requests that tests run more quickly, but its functionality
	// is provided by test writers themselves. The testing package is just its
	// home. The all.bash installation script sets it to make installation more
	// efficient, but by default the flag is off so a plain "go test" will do a
	// full test of the package.
	short = flag.Bool("test.short", false, "run smaller test suite to save time")

	// The failfast flag requests that test execution stop after the first test failure.
	failFast = flag.Bool("test.failfast", false, "do not start new tests after the first test failure")

	// The directory in which to create profile files and the like. When run from
	// "go test", the binary always runs in the source directory for the package;
	// this flag lets "go test" tell the binary to write the files in the directory where
	// the "go test" command is run.
	outputDir = flag.String("test.outputdir", "", "write profiles to `dir`")
	// Report as tests are run; default is silent for success.
	chatty = flag.Bool("test.v", false, "verbose: print additional output")
	count = flag.Uint("test.count", 1, "run tests and benchmarks `n` times")
	coverProfile = flag.String("test.coverprofile", "", "write a coverage profile to `file`")
	matchList = flag.String("test.list", "", "list tests, examples, and benchmarks matching `regexp` then exit")
	match = flag.String("test.run", "", "run only tests and examples matching `regexp`")
	memProfile = flag.String("test.memprofile", "", "write an allocation profile to `file`")
	memProfileRate = flag.Int("test.memprofilerate", 0, "set memory allocation profiling `rate` (see runtime.MemProfileRate)")
	cpuProfile = flag.String("test.cpuprofile", "", "write a cpu profile to `file`")
	blockProfile = flag.String("test.blockprofile", "", "write a goroutine blocking profile to `file`")
	blockProfileRate = flag.Int("test.blockprofilerate", 1, "set blocking profile `rate` (see runtime.SetBlockProfileRate)")
	mutexProfile = flag.String("test.mutexprofile", "", "write a mutex contention profile to the named file after execution")
	mutexProfileFraction = flag.Int("test.mutexprofilefraction", 1, "if >= 0, calls runtime.SetMutexProfileFraction()")
	panicOnExit0 = flag.Bool("test.paniconexit0", false, "panic on call to os.Exit(0)")
	traceFile = flag.String("test.trace", "", "write an execution trace to `file`")
	timeout = flag.Duration("test.timeout", 0, "panic test binary after duration `d` (default 0, timeout disabled)")
	cpuListStr = flag.String("test.cpu", "", "comma-separated `list` of cpu counts to run each test with")
	parallel = flag.Int("test.parallel", runtime.GOMAXPROCS(0), "run at most `n` tests in parallel")
	testlog = flag.String("test.testlogfile", "", "write test action log to `file` (for use only by cmd/go)")
	shuffle = flag.String("test.shuffle", "off", "randomize the execution order of tests and benchmarks")
}

var (
	// Flags, registered during Init.
	short                *bool
	failFast             *bool
	outputDir            *string
	chatty               *bool
	count                *uint
	coverProfile         *string
	matchList            *string
	match                *string
	memProfile           *string
	memProfileRate       *int
	cpuProfile           *string
	blockProfile         *string
	blockProfileRate     *int
	mutexProfile         *string
	mutexProfileFraction *int
	panicOnExit0         *bool
	traceFile            *string
	timeout              *time.Duration
	cpuListStr           *string
	parallel             *int
	shuffle              *string
	testlog              *string

	haveExamples bool // are there examples?

	cpuList     []int
	testlogFile *os.File

	numFailed uint32 // number of test failures
)

type chattyPrinter struct {
	w          io.Writer
	lastNameMu sync.Mutex // guards lastName
	lastName   string     // last printed test name in chatty mode
}

func newChattyPrinter(w io.Writer) *chattyPrinter {
	return &chattyPrinter{w: w}
}

// Updatef prints a message about the status of the named test to w.
//
// The formatted message must include the test name itself.
func (p *chattyPrinter) Updatef(testName, format string, args ...interface{}) {
	p.lastNameMu.Lock()
	defer p.lastNameMu.Unlock()

	// Since the message already implies an association with a specific new test,
	// we don't need to check what the old test name was or log an extra CONT line
	// for it. (We're updating it anyway, and the current message already includes
	// the test name.)
	p.lastName = testName

	fmt.Printf(format, args...)
}

// Printf prints a message, generated by the named test, that does not
// necessarily mention that tests's name itself.
func (p *chattyPrinter) Printf(testName, format string, args ...interface{}) {
	p.lastNameMu.Lock()
	defer p.lastNameMu.Unlock()

	if p.lastName == "" {
		p.lastName = testName
	} else if p.lastName != testName {
		fmt.Printf("=== CONT  %s\n", testName)
		p.lastName = testName
	}

	fmt.Printf(format, args...)
}

// The maximum number of stack frames to go through when skipping helper functions for
// the purpose of decorating log messages.
const maxStackLen = 50

// common holds the elements common between T and B and
// captures common methods such as Errorf.
type common struct {
	mu          sync.RWMutex         // guards this group of fields
	output      []byte               // Output generated by test or benchmark.
	w           io.Writer            // For flushToParent.
	ran         bool                 // Test or benchmark (or one of its subtests) was executed.
	failed      bool                 // Test or benchmark has failed.
	skipped     bool                 // Test or benchmark has been skipped.
	done        bool                 // Test is finished and all subtests have completed.
	helperPCs   map[uintptr]struct{} // functions to be skipped when writing file/line info
	helperNames map[string]struct{}  // helperPCs converted to function names
	cleanups    []func()             // optional functions to be called at the end of the test
	cleanupName string               // Name of the cleanup function.
	cleanupPc   []uintptr            // The stack trace at the point where Cleanup was called.
	finished    bool                 // Test function has completed.
	inFuzzFn    bool                 // Whether the fuzz target, if this is one, is running.

	chatty     *chattyPrinter // A copy of chattyPrinter, if the chatty flag is set.
	bench      bool           // Whether the current test is a benchmark.
	hasSub     int32          // Written atomically.
	raceErrors int            // Number of races detected during test.
	runner     string         // Function name of tRunner running the test.

	parent   *common
	level    int       // Nesting depth of test or benchmark.
	creator  []uintptr // If level > 0, the stack trace at the point where the parent called t.Run.
	name     string    // Name of test or benchmark.
	start    time.Time // Time test or benchmark started
	duration time.Duration
	barrier  chan bool // To signal parallel subtests they may start. Nil when T.Parallel is not present (B) or not usable (when fuzzing).
	signal   chan bool // To signal a test is done.
	sub      []*T      // Queue of subtests to be run in parallel.

	tempDirMu  sync.Mutex
	tempDir    string
	tempDirErr error
	tempDirSeq int32
}

// Short reports whether the -test.short flag is set.
func Short() bool {
	if short == nil {
		panic("testing: Short called before Init")
	}
	// Catch code that calls this from TestMain without first calling flag.Parse.
	if !flag.Parsed() {
		panic("testing: Short called before Parse")
	}

	return *short
}

// Verbose reports whether the -test.v flag is set.
func Verbose() bool {
	// Same as in Short.
	if chatty == nil {
		panic("testing: Verbose called before Init")
	}
	if !flag.Parsed() {
		panic("testing: Verbose called before Parse")
	}
	return *chatty
}

func (c *common) checkFuzzFn(name string) {
	if c.inFuzzFn {
		panic(fmt.Sprintf("testing: f.%s was called inside the fuzz target, use t.%s instead", name, name))
	}
}

// frameSkip searches, starting after skip frames, for the first caller frame
// in a function not marked as a helper and returns that frame.
// The search stops if it finds a tRunner function that
// was the entry point into the test and the test is not a subtest.
// This function must be called with c.mu held.
func (c *common) frameSkip(skip int) runtime.Frame {
	// If the search continues into the parent test, we'll have to hold
	// its mu temporarily. If we then return, we need to unlock it.
	shouldUnlock := false
	defer func() {
		if shouldUnlock {
			c.mu.Unlock()
		}
	}()
	var pc [maxStackLen]uintptr
	// Skip two extra frames to account for this function
	// and runtime.Callers itself.
	n := runtime.Callers(skip+2, pc[:])
	if n == 0 {
		panic("testing: zero callers found")
	}
	frames := runtime.CallersFrames(pc[:n])
	var firstFrame, prevFrame, frame runtime.Frame
	for more := true; more; prevFrame = frame {
		frame, more = frames.Next()
		if frame.Function == "runtime.gopanic" {
			continue
		}
		if frame.Function == c.cleanupName {
			frames = runtime.CallersFrames(c.cleanupPc)
			continue
		}
		if firstFrame.PC == 0 {
			firstFrame = frame
		}
		if frame.Function == c.runner {
			// We've gone up all the way to the tRunner calling
			// the test function (so the user must have
			// called tb.Helper from inside that test function).
			// If this is a top-level test, only skip up to the test function itself.
			// If we're in a subtest, continue searching in the parent test,
			// starting from the point of the call to Run which created this subtest.
			if c.level > 1 {
				frames = runtime.CallersFrames(c.creator)
				parent := c.parent
				// We're no longer looking at the current c after this point,
				// so we should unlock its mu, unless it's the original receiver,
				// in which case our caller doesn't expect us to do that.
				if shouldUnlock {
					c.mu.Unlock()
				}
				c = parent
				// Remember to unlock c.mu when we no longer need it, either
				// because we went up another nesting level, or because we
				// returned.
				shouldUnlock = true
				c.mu.Lock()
				continue
			}
			return prevFrame
		}
		// If more helper PCs have been added since we last did the conversion
		if c.helperNames == nil {
			c.helperNames = make(map[string]struct{})
			for pc := range c.helperPCs {
				c.helperNames[pcToName(pc)] = struct{}{}
			}
		}
		if _, ok := c.helperNames[frame.Function]; !ok {
			// Found a frame that wasn't inside a helper function.
			return frame
		}
	}
	return firstFrame
}

// decorate prefixes the string with the file and line of the call site
// and inserts the final newline if needed and indentation spaces for formatting.
// This function must be called with c.mu held.
func (c *common) decorate(s string, skip int) string {
	frame := c.frameSkip(skip)
	file := frame.File
	line := frame.Line
	if file != "" {
		// Truncate file name at last file name separator.
		if index := strings.LastIndex(file, "/"); index >= 0 {
			file = file[index+1:]
		} else if index = strings.LastIndex(file, "\\"); index >= 0 {
			file = file[index+1:]
		}
	} else {
		file = "???"
	}
	if line == 0 {
		line = 1
	}
	buf := new(strings.Builder)
	// Every line is indented at least 4 spaces.
	buf.WriteString("    ")
	fmt.Fprintf(buf, "%s:%d: ", file, line)
	lines := strings.Split(s, "\n")
	if l := len(lines); l > 1 && lines[l-1] == "" {
		lines = lines[:l-1]
	}
	for i, line := range lines {
		if i > 0 {
			// Second and subsequent lines are indented an additional 4 spaces.
			buf.WriteString("\n        ")
		}
		buf.WriteString(line)
	}
	buf.WriteByte('\n')
	return buf.String()
}

// flushToParent writes c.output to the parent after first writing the header
// with the given format and arguments.
func (c *common) flushToParent(testName, format string, args ...interface{}) {
	p := c.parent
	p.mu.Lock()
	defer p.mu.Unlock()

	c.mu.Lock()
	defer c.mu.Unlock()

	if len(c.output) > 0 {
		format += "%s"
		args = append(args[:len(args):len(args)], c.output)
		c.output = c.output[:0] // but why?
	}

	if c.chatty != nil && p.w == c.chatty.w {
		// We're flushing to the actual output, so track that this output is
		// associated with a specific test (and, specifically, that the next output
		// is *not* associated with that test).
		//
		// Moreover, if c.output is non-empty it is important that this write be
		// atomic with respect to the output of other tests, so that we don't end up
		// with confusing '=== CONT' lines in the middle of our '--- PASS' block.
		// Neither humans nor cmd/test2json can parse those easily.
		// (See https://golang.org/issue/40771.)
		c.chatty.Updatef(testName, format, args...)
	} else {
		// We're flushing to the output buffer of the parent test, which will
		// itself follow a test-name header when it is finally flushed to stdout.
		fmt.Printf(format, args...)
	}
}

type indenter struct {
	c *common
}

func (w indenter) Write(b []byte) (n int, err error) {
	n = len(b)
	for len(b) > 0 {
		end := bytes.IndexByte(b, '\n')
		if end == -1 {
			end = len(b)
		} else {
			end++
		}
		// An indent of 4 spaces will neatly align the dashes with the status
		// indicator of the parent.
		const indent = "    "
		w.c.output = append(w.c.output, indent...)
		w.c.output = append(w.c.output, b[:end]...)
		b = b[end:]
	}
	return
}

// fmtDuration returns a string representing d in the form "87.00s".
func fmtDuration(d time.Duration) string {
	return fmt.Sprintf("%.2fs", d.Seconds())
}

// TB is the interface common to T, B, and F.
type TB interface {
	Cleanup(func())
	Error(args ...interface{})
	Errorf(format string, args ...interface{})
	Fail()
	FailNow()
	Failed() bool
	Fatal(args ...interface{})
	Fatalf(format string, args ...interface{})
	Helper()
	Log(args ...interface{})
	Logf(format string, args ...interface{})
	Name() string
	Setenv(key, value string)
	Skip(args ...interface{})
	SkipNow()
	Skipf(format string, args ...interface{})
	Skipped() bool

	// A private method to prevent users implementing the
	// interface and so future additions to it will not
	// violate Go 1 compatibility.
	private()
}

var _ TB = (*T)(nil)

// T is a type passed to Test functions to manage test state and support formatted test logs.
//
// A test ends when its Test function returns or calls any of the methods
// FailNow, Fatal, Fatalf, SkipNow, Skip, or Skipf. Those methods, as well as
// the Parallel method, must be called only from the goroutine running the
// Test function.
//
// The other reporting methods, such as the variations of Log and Error,
// may be called simultaneously from multiple goroutines.
type T struct {
	common
	isParallel bool
	isEnvSet   bool
	context    *testContext // For running tests and subtests.
}

func (c *common) private() {}

// Name returns the name of the running (sub-) test or benchmark.
//
// The name will include the name of the test along with the names of
// any nested sub-tests. If two sibling sub-tests have the same name,
// Name will append a suffix to guarantee the returned name is unique.
func (c *common) Name() string {
	return c.name
}

func (c *common) setRan() {
	if c.parent != nil {
		c.parent.setRan()
	}
	c.mu.Lock()
	defer c.mu.Unlock()
	c.ran = true
}

// Fail marks the function as having failed but continues execution.
func (c *common) Fail() {
	if c.parent != nil {
		c.parent.Fail()
	}
	c.mu.Lock()
	defer c.mu.Unlock()
	// c.done needs to be locked to synchronize checks to c.done in parent tests.
	if c.done {
		panic("Fail in goroutine after " + c.name + " has completed")
	}
	c.failed = true
}

// Failed reports whether the function has failed.
func (c *common) Failed() bool {
	c.mu.RLock()
	failed := c.failed
	c.mu.RUnlock()
	return failed || c.raceErrors+race.Errors() > 0
}

// FailNow marks the function as having failed and stops its execution
// by calling runtime.Goexit (which then runs all deferred calls in the
// current goroutine).
// Execution will continue at the next test or benchmark.
// FailNow must be called from the goroutine running the
// test or benchmark function, not from other goroutines
// created during the test. Calling FailNow does not stop
// those other goroutines.
func (c *common) FailNow() {
	c.checkFuzzFn("FailNow")
	c.Fail()

	// Calling runtime.Goexit will exit the goroutine, which
	// will run the deferred functions in this goroutine,
	// which will eventually run the deferred lines in tRunner,
	// which will signal to the test loop that this test is done.
	//
	// A previous version of this code said:
	//
	//	c.duration = ...
	//	c.signal <- c.self
	//	runtime.Goexit()
	//
	// This previous version duplicated code (those lines are in
	// tRunner no matter what), but worse the goroutine teardown
	// implicit in runtime.Goexit was not guaranteed to complete
	// before the test exited. If a test deferred an important cleanup
	// function (like removing temporary files), there was no guarantee
	// it would run on a test failure. Because we send on c.signal during
	// a top-of-stack deferred function now, we know that the send
	// only happens after any other stacked defers have completed.
	c.mu.Lock()
	c.finished = true
	c.mu.Unlock()
	runtime.Goexit()
}

// log generates the output. It's always at the same stack depth.
func (c *common) log(s string) {
	c.logDepth(s, 3) // logDepth + log + public function
}

// logDepth generates the output at an arbitrary stack depth.
func (c *common) logDepth(s string, depth int) {
	c.mu.Lock()
	defer c.mu.Unlock()
	if c.done {
		// This test has already finished. Try and log this message
		// with our parent. If we don't have a parent, panic.
		for parent := c.parent; parent != nil; parent = parent.parent {
			parent.mu.Lock()
			defer parent.mu.Unlock()
			if !parent.done {
				parent.output = append(parent.output, parent.decorate(s, depth+1)...)
				return
			}
		}
		panic("Log in goroutine after " + c.name + " has completed: " + s)
	} else {
		if c.chatty != nil {
			if c.bench {
				// Benchmarks don't print === CONT, so we should skip the test
				// printer and just print straight to stdout.
				fmt.Print(c.decorate(s, depth+1))
			} else {
				c.chatty.Printf(c.name, "%s", c.decorate(s, depth+1))
			}

			return
		}
		c.output = append(c.output, c.decorate(s, depth+1)...)
	}
}

// Log formats its arguments using default formatting, analogous to Println,
// and records the text in the error log. For tests, the text will be printed only if
// the test fails or the -test.v flag is set. For benchmarks, the text is always
// printed to avoid having performance depend on the value of the -test.v flag.
func (c *common) Log(args ...interface{}) {
	c.checkFuzzFn("Log")
	c.log(fmt.Sprintln(args...))
}

// Logf formats its arguments according to the format, analogous to Printf, and
// records the text in the error log. A final newline is added if not provided. For
// tests, the text will be printed only if the test fails or the -test.v flag is
// set. For benchmarks, the text is always printed to avoid having performance
// depend on the value of the -test.v flag.
func (c *common) Logf(format string, args ...interface{}) {
	c.checkFuzzFn("Logf")
	c.log(fmt.Sprintf(format, args...))
}

// Error is equivalent to Log followed by Fail.
func (c *common) Error(args ...interface{}) {
	c.checkFuzzFn("Error")
	c.log(fmt.Sprintln(args...))
	c.Fail()
}

// Errorf is equivalent to Logf followed by Fail.
func (c *common) Errorf(format string, args ...interface{}) {
	c.checkFuzzFn("Errorf")
	c.log(fmt.Sprintf(format, args...))
	c.Fail()
}

// Fatal is equivalent to Log followed by FailNow.
func (c *common) Fatal(args ...interface{}) {
	c.checkFuzzFn("Fatal")
	c.log(fmt.Sprintln(args...))
	c.FailNow()
}

// Fatalf is equivalent to Logf followed by FailNow.
func (c *common) Fatalf(format string, args ...interface{}) {
	c.checkFuzzFn("Fatalf")
	c.log(fmt.Sprintf(format, args...))
	c.FailNow()
}

// Skip is equivalent to Log followed by SkipNow.
func (c *common) Skip(args ...interface{}) {
	c.checkFuzzFn("Skip")
	c.log(fmt.Sprintln(args...))
	c.SkipNow()
}

// Skipf is equivalent to Logf followed by SkipNow.
func (c *common) Skipf(format string, args ...interface{}) {
	c.checkFuzzFn("Skipf")
	c.log(fmt.Sprintf(format, args...))
	c.SkipNow()
}

// SkipNow marks the test as having been skipped and stops its execution
// by calling runtime.Goexit.
// If a test fails (see Error, Errorf, Fail) and is then skipped,
// it is still considered to have failed.
// Execution will continue at the next test or benchmark. See also FailNow.
// SkipNow must be called from the goroutine running the test, not from
// other goroutines created during the test. Calling SkipNow does not stop
// those other goroutines.
func (c *common) SkipNow() {
	c.checkFuzzFn("SkipNow")
	c.mu.Lock()
	c.skipped = true
	c.finished = true
	c.mu.Unlock()
	runtime.Goexit()
}

// Skipped reports whether the test was skipped.
func (c *common) Skipped() bool {
	c.mu.RLock()
	defer c.mu.RUnlock()
	return c.skipped
}

// Helper marks the calling function as a test helper function.
// When printing file and line information, that function will be skipped.
// Helper may be called simultaneously from multiple goroutines.
func (c *common) Helper() {
	c.mu.Lock()
	defer c.mu.Unlock()
	if c.helperPCs == nil {
		c.helperPCs = make(map[uintptr]struct{})
	}
	// repeating code from callerName here to save walking a stack frame
	var pc [1]uintptr
	n := runtime.Callers(2, pc[:]) // skip runtime.Callers + Helper
	if n == 0 {
		panic("testing: zero callers found")
	}
	if _, found := c.helperPCs[pc[0]]; !found {
		c.helperPCs[pc[0]] = struct{}{}
		c.helperNames = nil // map will be recreated next time it is needed
	}
}

// Cleanup registers a function to be called when the test (or subtest) and all its
// subtests complete. Cleanup functions will be called in last added,
// first called order.
func (c *common) Cleanup(f func()) {
	c.checkFuzzFn("Cleanup")
	var pc [maxStackLen]uintptr
	// Skip two extra frames to account for this function and runtime.Callers itself.
	n := runtime.Callers(2, pc[:])
	cleanupPc := pc[:n]

	fn := func() {
		defer func() {
			c.mu.Lock()
			defer c.mu.Unlock()
			c.cleanupName = ""
			c.cleanupPc = nil
		}()

		name := callerName(0)
		c.mu.Lock()
		c.cleanupName = name
		c.cleanupPc = cleanupPc
		c.mu.Unlock()

		f()
	}

	c.mu.Lock()
	defer c.mu.Unlock()
	c.cleanups = append(c.cleanups, fn)
}

// removeAll is like os.RemoveAll, but retries Windows "Access is denied."
// errors up to an arbitrary timeout.
//
// Those errors have been known to occur spuriously on at least the
// windows-amd64-2012 builder (https://go.dev/issue/50051), and can only occur
// legitimately if the test leaves behind a temp file that either is still open
// or the test otherwise lacks permission to delete. In the case of legitimate
// failures, a failing test may take a bit longer to fail, but once the test is
// fixed the extra latency will go away.
func removeAll(path string) error {
	const arbitraryTimeout = 2 * time.Second
	var (
		start     time.Time
		nextSleep = 1 * time.Millisecond
	)
	for {
		err := os.RemoveAll(path)
		if start.IsZero() {
			start = time.Now()
		} else if d := time.Since(start) + nextSleep; d >= arbitraryTimeout {
			return err
		}
		time.Sleep(nextSleep)
		nextSleep += time.Duration(rand.Int63n(int64(nextSleep)))
	}
}

// Setenv calls os.Setenv(key, value) and uses Cleanup to
// restore the environment variable to its original value
// after the test.
//
// This cannot be used in parallel tests.
func (c *common) Setenv(key, value string) {
	c.checkFuzzFn("Setenv")
	prevValue, ok := os.LookupEnv(key)

	if err := os.Setenv(key, value); err != nil {
		c.Fatalf("cannot set environment variable: %v", err)
	}

	if ok {
		c.Cleanup(func() {
			os.Setenv(key, prevValue)
		})
	} else {
		c.Cleanup(func() {
			os.Unsetenv(key)
		})
	}
}

// panicHanding is an argument to runCleanup.
type panicHandling int

const (
	normalPanic panicHandling = iota
	recoverAndReturnPanic
)

// runCleanup is called at the end of the test.
// If catchPanic is true, this will catch panics, and return the recovered
// value if any.
func (c *common) runCleanup(ph panicHandling) (panicVal interface{}) {
	if ph == recoverAndReturnPanic {
		defer func() {
			panicVal = recover()
		}()
	}

	// Make sure that if a cleanup function panics,
	// we still run the remaining cleanup functions.
	defer func() {
		c.mu.Lock()
		recur := len(c.cleanups) > 0
		c.mu.Unlock()
		if recur {
			c.runCleanup(normalPanic)
		}
	}()

	for {
		var cleanup func()
		c.mu.Lock()
		if len(c.cleanups) > 0 {
			last := len(c.cleanups) - 1
			cleanup = c.cleanups[last]
			c.cleanups = c.cleanups[:last]
		}
		c.mu.Unlock()
		if cleanup == nil {
			return nil
		}
		cleanup()
	}
}

// callerName gives the function name (qualified with a package path)
// for the caller after skip frames (where 0 means the current function).
func callerName(skip int) string {
	var pc [1]uintptr
	n := runtime.Callers(skip+2, pc[:]) // skip + runtime.Callers + callerName
	if n == 0 {
		panic("testing: zero callers found")
	}
	return pcToName(pc[0])
}

func pcToName(pc uintptr) string {
	pcs := []uintptr{pc}
	frames := runtime.CallersFrames(pcs)
	frame, _ := frames.Next()
	return frame.Function
}

// Parallel signals that this test is to be run in parallel with (and only with)
// other parallel tests. When a test is run multiple times due to use of
// -test.count or -test.cpu, multiple instances of a single test never run in
// parallel with each other.
func (t *T) Parallel() {
	if t.isParallel {
		panic("testing: t.Parallel called multiple times")
	}
	if t.isEnvSet {
		panic("testing: t.Parallel called after t.Setenv; cannot set environment variables in parallel tests")
	}
	t.isParallel = true
	if t.parent.barrier == nil {
		// T.Parallel has no effect when fuzzing.
		// Multiple processes may run in parallel, but only one input can run at a
		// time per process so we can attribute crashes to specific inputs.
		return
	}

	// We don't want to include the time we spend waiting for serial tests
	// in the test duration. Record the elapsed time thus far and reset the
	// timer afterwards.
	t.duration += time.Since(t.start)

	// Add to the list of tests to be released by the parent.
	t.parent.sub = append(t.parent.sub, t)
	t.raceErrors += race.Errors()

	if t.chatty != nil {
		// Unfortunately, even though PAUSE indicates that the named test is *no
		// longer* running, cmd/test2json interprets it as changing the active test
		// for the purpose of log parsing. We could fix cmd/test2json, but that
		// won't fix existing deployments of third-party tools that already shell
		// out to older builds of cmd/test2json — so merely fixing cmd/test2json
		// isn't enough for now.
		t.chatty.Updatef(t.name, "=== PAUSE %s\n", t.name)
	}

	t.signal <- true   // Release calling test.
	<-t.parent.barrier // Wait for the parent test to complete.
	t.context.waitParallel()

	if t.chatty != nil {
		t.chatty.Updatef(t.name, "=== CONT  %s\n", t.name)
	}

	t.start = time.Now()
	t.raceErrors += -race.Errors()
}

// Setenv calls os.Setenv(key, value) and uses Cleanup to
// restore the environment variable to its original value
// after the test.
//
// This cannot be used in parallel tests.
func (t *T) Setenv(key, value string) {
	if t.isParallel {
		panic("testing: t.Setenv called after t.Parallel; cannot set environment variables in parallel tests")
	}

	t.isEnvSet = true

	t.common.Setenv(key, value)
}

// InternalTest is an internal type but exported because it is cross-package;
// it is part of the implementation of the "go test" command.
type InternalTest struct {
	Name string
	F    func(*T)
}

var errNilPanicOrGoexit = errors.New("test executed panic(nil) or runtime.Goexit")

func tRunner(t *T, fn func(t *T)) {
	t.runner = callerName(0)

	// When this goroutine is done, either because fn(t)
	// returned normally or because a test failure triggered
	// a call to runtime.Goexit, record the duration and send
	// a signal saying that the test is done.
	defer func() {
		if t.Failed() {
			atomic.AddUint32(&numFailed, 1)
		}

		if t.raceErrors+race.Errors() > 0 {
			t.Errorf("race detected during execution of test")
		}

		// Check if the test panicked or Goexited inappropriately.
		//
		// If this happens in a normal test, print output but continue panicking.
		// tRunner is called in its own goroutine, so this terminates the process.
		//
		// If this happens while fuzzing, recover from the panic and treat it like a
		// normal failure. It's important that the process keeps running in order to
		// find short inputs that cause panics.
		err := recover()
		signal := true

		t.mu.RLock()
		finished := t.finished
		t.mu.RUnlock()
		if !finished && err == nil {
			err = errNilPanicOrGoexit
			for p := t.parent; p != nil; p = p.parent {
				p.mu.RLock()
				finished = p.finished
				p.mu.RUnlock()
				if finished {
					t.Errorf("%v: subtest may have called FailNow on a parent test", err)
					err = nil
					signal = false
					break
				}
			}
		}

		if err != nil && t.context.isFuzzing {
			prefix := "panic: "
			if err == errNilPanicOrGoexit {
				prefix = ""
			}
			t.Errorf("%s%s\n%s\n", prefix, err, string(debug.Stack()))
			t.mu.Lock()
			t.finished = true
			t.mu.Unlock()
			err = nil
		}

		// Use a deferred call to ensure that we report that the test is
		// complete even if a cleanup function calls t.FailNow. See issue 41355.
		didPanic := false
		defer func() {
			if didPanic {
				return
			}
			if err != nil {
				panic(err)
			}
			// Only report that the test is complete if it doesn't panic,
			// as otherwise the test binary can exit before the panic is
			// reported to the user. See issue 41479.
			t.signal <- signal
		}()

		doPanic := func(err interface{}) {
			t.Fail()
			if r := t.runCleanup(recoverAndReturnPanic); r != nil {
				t.Logf("cleanup panicked with %v", r)
			}
			// Flush the output log up to the root before dying.
			for root := &t.common; root.parent != nil; root = root.parent {
				root.mu.Lock()
				root.duration += time.Since(root.start)
				d := root.duration
				root.mu.Unlock()
				root.flushToParent(root.name, "--- FAIL: %s (%s)\n", root.name, fmtDuration(d))
				if r := root.parent.runCleanup(recoverAndReturnPanic); r != nil {
					fmt.Fprintf(root.parent.w, "cleanup panicked with %v", r)
				}
			}
			didPanic = true
			panic(err)
		}
		if err != nil {
			doPanic(err)
		}

		t.duration += time.Since(t.start)

		if len(t.sub) > 0 {
			// Run parallel subtests.
			// Decrease the running count for this test.
			t.context.release()
			// Release the parallel subtests.
			close(t.barrier)
			// Wait for subtests to complete.
			for _, sub := range t.sub {
				<-sub.signal
			}
			cleanupStart := time.Now()
			err := t.runCleanup(recoverAndReturnPanic)
			t.duration += time.Since(cleanupStart)
			if err != nil {
				doPanic(err)
			}
			if !t.isParallel {
				// Reacquire the count for sequential tests. See comment in Run.
				t.context.waitParallel()
			}
		} else if t.isParallel {
			// Only release the count for this test if it was run as a parallel
			// test. See comment in Run method.
			t.context.release()
		}
		t.report() // Report after all subtests have finished.

		// Do not lock t.done to allow race detector to detect race in case
		// the user does not appropriately synchronize a goroutine.
		t.done = true
		if t.parent != nil && atomic.LoadInt32(&t.hasSub) == 0 {
			t.setRan()
		}
	}()
	defer func() {
		if len(t.sub) == 0 {
			t.runCleanup(normalPanic)
		}
	}()

	t.start = time.Now()
	t.raceErrors = -race.Errors()
	fn(t)

	// code beyond here will not be executed when FailNow is invoked
	t.mu.Lock()
	t.finished = true
	t.mu.Unlock()
}

// Run runs f as a subtest of t called name. It runs f in a separate goroutine
// and blocks until f returns or calls t.Parallel to become a parallel test.
// Run reports whether f succeeded (or at least did not fail before calling t.Parallel).
//
// Run may be called simultaneously from multiple goroutines, but all such calls
// must return before the outer test function for t returns.
func (t *T) Run(name string, f func(t *T)) bool {
	atomic.StoreInt32(&t.hasSub, 1)
	testName, ok, _ := t.context.match.fullName(&t.common, name)
	if !ok || shouldFailFast() {
		return true
	}
	// Record the stack trace at the point of this call so that if the subtest
	// function - which runs in a separate stack - is marked as a helper, we can
	// continue walking the stack into the parent test.
	var pc [maxStackLen]uintptr
	n := runtime.Callers(2, pc[:])
	t = &T{
		common: common{
			barrier: make(chan bool),
			signal:  make(chan bool, 1),
			name:    testName,
			parent:  &t.common,
			level:   t.level + 1,
			creator: pc[:n],
			chatty:  t.chatty,
		},
		context: t.context,
	}
	t.w = indenter{&t.common}

	if t.chatty != nil {
		t.chatty.Updatef(t.name, "=== RUN   %s\n", t.name)
	}
	// Instead of reducing the running count of this test before calling the
	// tRunner and increasing it afterwards, we rely on tRunner keeping the
	// count correct. This ensures that a sequence of sequential tests runs
	// without being preempted, even when their parent is a parallel test. This
	// may especially reduce surprises if *parallel == 1.
	tRunner(t, f)
	if !<-t.signal {
		// At this point, it is likely that FailNow was called on one of the
		// parent tests by one of the subtests. Continue aborting up the chain.
		runtime.Goexit()
	}
	return !t.failed
}

// Deadline reports the time at which the test binary will have
// exceeded the timeout specified by the -timeout flag.
//
// The ok result is false if the -timeout flag indicates “no timeout” (0).
func (t *T) Deadline() (deadline time.Time, ok bool) {
	deadline = t.context.deadline
	return deadline, !deadline.IsZero()
}

// testContext holds all fields that are common to all tests. This includes
// synchronization primitives to run at most *parallel tests.
type testContext struct {
	match    *matcher
	deadline time.Time

	// isFuzzing is true in the context used when generating random inputs
	// for fuzz targets. isFuzzing is false when running normal tests and
	// when running fuzz tests as unit tests (without -fuzz or when -fuzz
	// does not match).
	isFuzzing bool

	mu sync.Mutex

	// Channel used to signal tests that are ready to be run in parallel.
	startParallel chan bool

	// running is the number of tests currently running in parallel.
	// This does not include tests that are waiting for subtests to complete.
	running int

	// numWaiting is the number tests waiting to be run in parallel.
	numWaiting int

	// maxParallel is a copy of the parallel flag.
	maxParallel int
}

func newTestContext(maxParallel int, m *matcher) *testContext {
	return &testContext{
		match:         m,
		startParallel: make(chan bool),
		maxParallel:   maxParallel,
		running:       1, // Set the count to 1 for the main (sequential) test.
	}
}

func (c *testContext) waitParallel() {
	c.mu.Lock()
	if c.running < c.maxParallel {
		c.running++
		c.mu.Unlock()
		return
	}
	c.numWaiting++
	c.mu.Unlock()
	<-c.startParallel
}

func (c *testContext) release() {
	c.mu.Lock()
	if c.numWaiting == 0 {
		c.running--
		c.mu.Unlock()
		return
	}
	c.numWaiting--
	c.mu.Unlock()
	c.startParallel <- true // Pick a waiting test to be run.
}

// No one should be using func Main anymore.
// See the doc comment on func Main and use MainStart instead.
var errMain = errors.New("testing: unexpected use of func Main")

type matchStringOnly func(pat, str string) (bool, error)

func (f matchStringOnly) MatchString(pat, str string) (bool, error)       { return f(pat, str) }
func (f matchStringOnly) StartCPUProfile(w io.Writer) error               { return errMain }
func (f matchStringOnly) StopCPUProfile()                                 {}
func (f matchStringOnly) WriteProfileTo(string, io.Writer, int) error     { return errMain }
func (f matchStringOnly) ImportPath() string                              { return "" }
func (f matchStringOnly) StartTestLog(io.Writer)                          {}
func (f matchStringOnly) StopTestLog() error                              { return errMain }
func (f matchStringOnly) SetPanicOnExit0(bool)                            {}
func (f matchStringOnly) CheckCorpus([]interface{}, []reflect.Type) error { return nil }
func (f matchStringOnly) ResetCoverage()                                  {}
func (f matchStringOnly) SnapshotCoverage()                               {}

func (t *T) report() {
	if t.parent == nil {
		return
	}
	dstr := fmtDuration(t.duration)
	format := "--- %s: %s (%s)\n"
	if t.Failed() {
		t.flushToParent(t.name, format, "FAIL", t.name, dstr)
	} else if t.chatty != nil {
		if t.Skipped() {
			t.flushToParent(t.name, format, "SKIP", t.name, dstr)
		} else {
			t.flushToParent(t.name, format, "PASS", t.name, dstr)
		}
	}
}

// toOutputDir returns the file name relocated, if required, to outputDir.
// Simple implementation to avoid pulling in path/filepath.
func toOutputDir(path string) string {
	if *outputDir == "" || path == "" {
		return path
	}
	// On Windows, it's clumsy, but we can be almost always correct
	// by just looking for a drive letter and a colon.
	// Absolute paths always have a drive letter (ignoring UNC).
	// Problem: if path == "C:A" and outputdir == "C:\Go" it's unclear
	// what to do, but even then path/filepath doesn't help.
	// TODO: Worth doing better? Probably not, because we're here only
	// under the management of go test.
	if runtime.GOOS == "windows" && len(path) >= 2 {
		letter, colon := path[0], path[1]
		if ('a' <= letter && letter <= 'z' || 'A' <= letter && letter <= 'Z') && colon == ':' {
			// If path starts with a drive letter we're stuck with it regardless.
			return path
		}
	}
	if os.IsPathSeparator(path[0]) {
		return path
	}
	return fmt.Sprintf("%s%c%s", *outputDir, os.PathSeparator, path)
}

func parseCpuList() {
	for _, val := range strings.Split(*cpuListStr, ",") {
		val = strings.TrimSpace(val)
		if val == "" {
			continue
		}
		cpu, err := strconv.Atoi(val)
		if err != nil || cpu <= 0 {
			fmt.Fprintf(os.Stderr, "testing: invalid value %q for -test.cpu\n", val)
			os.Exit(1)
		}
		cpuList = append(cpuList, cpu)
	}
	if cpuList == nil {
		cpuList = append(cpuList, runtime.GOMAXPROCS(-1))
	}
}

func shouldFailFast() bool {
	return *failFast && atomic.LoadUint32(&numFailed) > 0
}

// =====================================================================

func TestSuite(name string, f func(t *T)) {
	Init()

	var buf bytes.Buffer
	var w io.Writer
	ctx := newTestContext(1, newMatcher(regexp.MatchString, "", ""))

	t := &T{
		common: common{
			signal: make(chan bool),
			chatty: newChattyPrinter(w),
			w:      &buf,
			name:   "test suite",
		},
		context: ctx,
	}

	t.Run(name, f)
}
