/*
* Copyright (c) 2001, 2024, Oracle and/or its affiliates.
 *
 * This file contains the C api to the Oracle Notification System (ONS)
 */

#ifndef _ORACLE_ONS_H
#define _ORACLE_ONS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @mainpage ONS API Documentation
 *
 * @section Overview Overview
 * This document provides detailed API and usage documentation for the C
 * version of ONS. The ONS documentation is broken into the following
 * sections:
 *   - @ref Init
 *   - @ref Error
 *   - @ref NodeList
 *   - @ref Notification
 *   - @ref Subscriber
 *   - @ref Publisher
 *   - @ref RPC
 *   - @ref BodyBlock
 *   - @ref Socket
 *   - @ref Threads
 *   - @ref Pool
 */
/**
 * @defgroup ONS  Oracle Notification Service
 *
 * @{
 *
 * Requires <b><tt>&lt;ons.h&gt;</tt></b>.
 */
/**
 * @defgroup Init  Startup and shutdown
 * @ingroup ONS
 *
 * @{
 */
/**
 * @defgroup Environment  Environment variables
 * @ingroup Init
 *
 * @{
 *
 * Common environment variables referenced when an ONS client is initialized.
 *
 * <b><tt>ORACLE_ONS_DEBUG</tt></b> Enables client trace output.
 *
 * <b><tt>ORACLE_ONS_LOGFILE</tt></b> Specifies the path for client error
 * and trace output. The default is stderr, and this value is overriden by
 * specifying a call back for error or trace output. The client will replace
 * the the first occurance of <i><tt>{PID}</tt></i> in the path with the
 * pid of the process.
 *
 * <b><tt>ORACLE_ONS_IGNORESCANVIP</tt></b> Only for initialization with a
 * configuration string (wconfig). The client will by default treat each
 * configured host entry in a node list as SCAN, but setting this environment
 * variable to "true" will disable this default behavior.
 *
 * <b><tt>ORACLE_ONS_USEWEBSOCKET</tt></b> Only for initialization with a
 * configuration string (wconfig). If "true" the client will use the websocket
 * protocol for connections to remote servers.
 *
 * <b><tt>ORACLE_ONS_WEBSOCKETPROXY</tt></b> Only for initialization with a
 * configuration string (wconfig). Specifies the 
 * <tt><b>&lt;host&gt;:&lt;port&gt;</b></tt> pair of the websocket proxy to
 * use when connecting to remote servers. Enables the websocket protocol
 * regardless of the presence or value of <tt>ORACLE_ONS_USEWEBSOCKET</tt>.
 * @}
 */

/**
 * The scope of the thread stack size setting
 */
typedef enum _ons_thread_scope
{
  /** All ONS client created threads */
  OnsThreadAll = 0,
  /** ONS client created internal threads */
  OnsThreadInternal,
 /** ONS client created call-back threads */
  OnsThreadCallBack
} ons_thread_scope;

/**
 * Sets the thread stack size for for the given thread scope for all subsequent
 * calls to initialize the ONS client. This call will have no affect on any
 * client (or clients) initialized before ons_thread_stacksize() is called.
 *
 * @param[in]  threadScope The scope for the threadStackSize.
 * @param[in]  threadStackSize The thread stack size the ONS client will use
 *             for all threads it creates. Absolutely <b>no</b> validation is
 *             performed for this value, and if it is invalid, ONS will still
 *             blindly attempt to create threads using it, and any thread
 *             creation failures will leave the ONS client in a non
 *             functional state.
 *
 * @see ons_init()
 * @see ons_init_woraclehome()
 * @see ons_init_woraclehome_ctx()
 * @see ons_init_wconfig()
 * @see ons_init_wconfig_ctx()
 * @see ons_init_wconfig_common_ctx()
 */
void ons_thread_stacksize(ons_thread_scope threadScope,
       unsigned int threadStackSize);

/**
 * Performs the setup required for the ONS client global context: initiates
 * connection to the ONS server, although it does not block waiting for
 * the server connection to complete.
 * Requires one of the following environment variables:
 * - ORACLE_BASE_HOME, ORACLE_CONFIG_HOME or ORACLE_HOME
 *
 * ORACLE_BASE_HOME, ORACLE_CONFIG_HOME and ORACLE_HOME are used for
 * obtaining internal configuration information for the ONS server.
 *
 * @return One of the following codes:
 *   -  0 - ONS initialized. Note that subsequent calls to ons_init() prior
 *          to a call to ons_shutdown() will always return 0, and the number
 *          of "nested" calls to ons_init() is recorded. The ONS client layer
 *          is only destroyed within the corresponding last call to
 *          ons_shutdown().
 *   - -1 - ONS initialization failed.
 *
 * @see ons_get_global_ctx()
 * @see ons_init_woraclehome()
 * @see ons_init_wconfig()
 * @see ons_shutdown()
 * @see ons_shutdown_wtimeout()
 */
int ons_init(void);

/**
 * Performs the setup required for the ONS client global context: initiates
 * connection to the ONS server, although it does not block waiting for
 * the server connection to complete.
 *
 * @param[in]  configPath The path to an ORACLE_HOME, ORACLE_BASE_HOME, or
 * ORACLE_CONFIG_HOME directory.
 *
 * @return One of the following codes:
 *   -  0 - ONS initialized. Note that subsequent calls to
 *          ons_init_woraclehome() prior to a call to ons_shutdown() will
 *          always return 0, and the number of "nested" calls to
 *          ons_init_woraclehome() is recorded. The ONS client layer is only
 *          destroyed within the corresponding last call to ons_shutdown().
 *   - -1 - ONS initialization failed.
 *
 * @see ons_get_global_ctx()
 * @see ons_init()
 * @see ons_init_wconfig()
 * @see ons_shutdown()
 * @see ons_shutdown_wtimeout()
 */
int ons_init_woraclehome(const char *configPath);

/**
 * Performs the setup required for a specific ONS client context that exists
 * and operates independent from any other ONS client context (including the
 * global context); multiple client contexts can be initialized, although it
 * is expected that the set of ONS servers specified by each is completely
 * unique. Initiates connections to the configured lists of ONS servers,
 * although it does not block waiting for the server connections to complete.
 * <p>
 * The caller must use the _ctx versions of the ONS APIs listed below to
 * shutdown the context, create subscribers or publisher, or get the
 * context's node-lists.
 * </p>
 *
 * @param[in]  configPath The path to an ORACLE_HOME, ORACLE_BASE_HOME, or
 * ORACLE_CONFIG_HOME directory.
 *
 * @return The ONS client context pointer, or NULL on error.
 *
 * @see ons_init_wconfig_ctx()
 * @see ons_init_wconfig_common_ctx()
 * @see ons_shutdown_wtimeout_ctx()
 * @see ons_subscriber_create_wtimeout_ctx()
 * @see ons_subscriber_create_async_ctx()
 * @see ons_publisher_create_ctx()
 * @see ons_nodelists_ctx()
 * @see ons_nodelist
 */
const void* ons_init_woraclehome_ctx(const char* configPath);

/** The ONS client was successfully initialized */
#define ONS_INIT            0

/** The ONS client failed to initialize because of a bad configuration */
#define ONS_BADCONFIG       -1

/** The ONS client failed to initialize because of an internal error */
#define ONS_ERROR           -2

/** The ONS client was already initialized */
#define ONS_ALREADYSTARTED  1

/**
 * Performs the setup required for the ONS client global context: initiates
 * connections to the configured lists of ONS servers, although it does not
 * block waiting for the server connections to complete.
 *
 * @param[in] config A string specifying the configuration of the client,
 * which must consist of ASCII UTF-8 characters.
 * The string is composed of multiple records, with each record consisting
 * of a <b><tt>key=value</tt></b> pair and terminated by a newline ('\\n')
 * character. Any Leading white space for each line is ignored, but all other
 * white space is considered part of the <b><tt>key</tt></b> or
 * <b><tt>value</tt></b>. If the first non-white space character in a line is
 * the character '#', the entire line is considered a comment and skipped
 * (only entire line comments are supported).
 * Aside from what is described above, there is no special handling of
 * characters, and they are used as is.
 * <p>
 * Valid  keys:
 * </p>
 * <ul>
 * <li><tt><b>walletfile</b></tt> See <tt><b>oracle.ons.walletfile</b></tt>
 * below.</li>
 * <li><tt><b>walletpassword</b></tt> See
 * <tt><b>oracle.ons.walletpassword</b></tt> below.</li>
 * <li><tt><b>remotetimeout</b></tt> See
 * <tt><b>oracle.ons.remotetimeout</b></tt> below.</li>
 * <li><tt><b>debug</b></tt> See <tt><b>oracle.ons.debug</b></tt>
 * below.</li>
 * <li><tt><b>shutdowntimeout</b></tt> See
 * <tt><b>oracle.ons.shutdowntimeout</b></tt> below.</li>
 * <li><tt><b>ignorescanvip</b></tt> See
 * <li><tt><b>oracle.ons.ignorescanvip</b></tt> below.</li>
 * <li><tt><b>usewebsocket</b></tt> See <tt><b>oracle.ons.usewebsocket</b></tt>
 * below.</li>
 * <li><tt><b>websocketproxy</b></tt> See
 * <tt><b>oracle.ons.websocketproxy</b></tt>
 * below.</li>
 * <li><tt><b>propertiesfile</b></tt> Specifies the path to a Java properties
 * file from which the values for the properties defined below will be
 * obtained. The entries in properties file must be in the<br>
 * <tt>key=value</tt><br>
 * format.</li>
 * <li><tt><b>nodes.&lt;id&gt;</b></tt> See
 * <tt><b>oracle.ons.nodes.&lt;id&gt;</b></tt> below.</li>
 * <li><tt><b>maxconnections.&lt;id&gt;</b></tt> See
 * <tt><b>oracle.ons.maxconnections.&lt;id&gt;</b></tt> below.</li>
 * <li><tt><b>active.&lt;id&gt;</b></tt> See
 * <tt><b>oracle.ons.active.&lt;id&gt;</b></tt> below.</li>
 * </ul>
 * <p>
 * Supported properties:
 * </p>
 * <p>
 * Global
 * </p>
 * <ul>
 * <li><tt><b>oracle.ons.debug</b></tt> If set to <tt><b>true</b></tt>, debug
 * trace output is enabled for the remote ONS client context. The trace output
 * is printed to stderr. The default is <tt><b>false</b></tt>.</li>
 * <li><tt><b>oracle.ons.shutdowntimeout</b></tt> The maximum time (in
 * seconds) to wait for the published notification queue to drain on
 * shutdown. The default is <tt><b>5</b></tt> seconds.</li>
 * <li><tt><b>oracle.ons.ignorescanvip</b></tt> Disable the SCAN VIP method
 * of resolving remote server addresses from the configured nodes list.
 * By default each address resolved for an entry in the nodes list is
 * treated itself as a separate remote server. Setting this property to
 * <tt><b>true</b></tt> causes each entry in the nodes list to be handled
 * as a single remote server entry.
 * The default is <tt><b>false</b></tt>.</li>
 * <li><tt><b>oracle.ons.remotetimeout</b></tt> The timeout period for
 * a connection to each remote server. If the remote server has not
 * responded within this timeout period, the connection will be closed.
 * The default is <tt><b>30</b></tt> seconds. A value of <tt><b>0</b></tt>
 * will disable server timeouts altogether; otherwise the minimum value
 * is 2.</li>
 * <li><tt><b>oracle.ons.walletfile</b></tt> The wallet file used for
 * establishing connections to the remote servers.</li>
 * <li><tt><b>oracle.ons.walletpassword</b></tt> The password used to
 * access the configured wallet file.</li>
 * <li><tt><b>oracle.ons.usewebsocket</b></tt> use the websocket protocol for
 * connections to all remote servers.</li>
 * <li><tt><b>oracle.ons.websocketproxy</b></tt> specifies the
 * protocol (optional), host and port of the websocket proxy to use
 * when connecting to remote servers. This property enables the websocket
 * protocol regardless of the presence or value of oracle.ons.websocketproxy.
 * The websocket proxy configuration takes one of the following formats:<br>
 * <table border=0>
 * <tr><td>&nbsp;</td><td><p>
 * <tt><b><i>&lt;protocol&gt;://</i>&lt;host&gt;:&lt;port&gt;</b></tt><br>
 * where<br>
 * <table border=0>
 * <tr><td>&nbsp;</td><td>
 * <tt><b>&lt;protocol&gt;</b></tt> is optional and must be one of
 * <tt><b>ws</b></tt>, <tt><b>wss</b></tt>,
 * <tt><b>http</b></tt>, <tt><b>https</b></tt>,
 * <tt><b>tcp</b></tt> or <tt><b>tcps</b></tt>.
 * The trailing '<tt><b>s</b></tt>' specifies SSL for each protocol.
 * Regardless of the root value the client will use HTTP/HTTPS to upgrade the
 * connection protocol to websocket (ws or wss). If no protocol is configured
 * then SSL is used by default if the client is configured with a wallet.
 * If <tt><b>&lt;protocol&gt;</b></tt> is configured it must be followed by
 * <tt><b>://</b></tt> and otherwise this suffix must not be used.
 * </td></tr>
 * <tr><td>&nbsp;</td><td>
 * <tt><b>&lt;host&gt;:&lt;port&gt;</b></tt> are required and specify the
 * host and port for the websocket proxy server.
 * </p></td></tr>
 * </table>
 * </td></tr>
 * <tr><td>&nbsp;</td><td>
 * <tt><b><i>(protocol=&lt;protocol&gt;)</i>(http_proxy=&lt;host&gt;)(http_proxy_port&lt;port&gt;)</b></tt><br>
 * &nbsp;&nbsp;&nbsp;&nbsp;or<br>
 * <tt><b><i>(protocol=&lt;protocol&gt;)</i>(https_proxy=&lt;host&gt;)(https_proxy_port&lt;port&gt;)</b></tt><br>
 * where the configured <tt>(&lt;name&gt;=&lt;value&gt;)</tt> pairs may be in
 * any order. Other <tt>(&lt;name&gt;=&lt;value&gt;)</tt> pairs may be present
 * and are ignored, as are nested parentheses. The following rules apply to
 * the expected <tt>(&lt;name&gt;=&lt;value&gt;)</tt> pairs:<br>
 * <table border=0>
 * <tr><td>&nbsp;</td><td>
 * <tt><b>&lt;protocol&gt;</b></tt> is optional and must be one of
 * <tt><b>ws</b></tt>, <tt><b>wss</b></tt>,
 * <tt><b>http</b></tt>, <tt><b>https</b></tt>,
 * <tt><b>tcp</b></tt> or <tt><b>tcps</b></tt>.
 * The trailing '<tt><b>s</b></tt>' specifies SSL for each protocol.
 * Regardless of the root value the client will use HTTP/HTTPS to upgrade the
 * connection protocol to websocket (ws or wss). If no protocol is configured
 * then SSL is used by default if the client is configured with a wallet.
 * </td></tr>
 * <tr><td>&nbsp;</td><td>
 * <tt><b>&lt;host&gt; and &lt;port&gt;</b></tt> are required and specify
 * the host and port for the websocket proxy server.
 * </td></tr>
 * </table>
 * </td></tr>
 * </table>
 * </li>
 * </ul>
 * <p>
 * Node lists
 * </p>
 * <ul>
 * <li><tt><b>oracle.ons.nodes.&lt;id&gt;</b></tt> A list of nodes
 * representing a unique topology of remote ONS servers.
 * <tt><b>&lt;id&gt;</b></tt> specifies a unique identifier for the node
 * list -- duplicate entries are ignored. The list of nodes configred in
 * any list must not include any nodes configured in any other list for
 * the same client or duplicate notifications will be sent and
 * delivered. <tt><b>&lt;id&gt;</b></tt> should be composed of alpha-numeric
 * characters.
 * <p>
 * The list of nodes is a comma separated list of node entries,
 * with each entry defined as a <tt><b>&lt;host&gt;:&lt;port&gt;</b></tt>
 * pair. <tt><b>&lt;host&gt;</b></tt> is a host name or IP address which
 * will be resolved into its network addresss or addresses; and if SCAN
 * is enabled (the default), each resolved address will be treated as a
 * separate remote ONS server entry in the node-list.
 * <tt><b>&lt;port&gt;</b></tt> is the numeric remote port value for the ONS
 * server, or servers if SCAN is enabled and <tt><b>&lt;host&gt;</b></tt>
 * resolves to multiple addresses.
 * </p>
 * </li>
 * <li><tt><b>oracle.ons.maxconnections.&lt;id&gt;</b></tt> Specifies the
 * maximum number of concurrent connections mainted with the ONS servers.
 * <tt><b>&lt;id&gt;</b></tt> specifies the node-list to which this
 * parameter applies.
 * The default is <tt><b>3</b></tt>.</li>
 * <li><tt><b>oracle.ons.active.&lt;id&gt;</b></tt> If <tt><b>true</b></tt>
 * the list is active and connections will automatically be established to 
 * the configured number of ONS servers.
 * If <tt><b>false</b></tt> the list is inactive and will only be used as
 * a fail over list in the event that no connections for an active list
 * can be established. An inactive list can only serve as a fail over
 * for one active list at a time, and once a single connection is
 * re-established on the active list, the fail-over list will revert to
 * being inactive. Note that only notifications published by the client
 * after a a list has failed over will be sent to the fail over list.
 * <tt><b>&lt;id&gt;</b></tt> specifies the node-list to which this
 * parameter applies.
 * The default is <tt><b>true</b></tt>.</li>
 * </ul>
 *
 * @return One of the following codes:
 *   - ONS_INIT - The client was successfully initialized.
 *   - ONS_BADCONFIG - The configuration string was invalid.
 *   - ONS_ERROR - An internal or system error occured.
 *   - ONS_ALREADYSTARTED - The ONS client has already been initialized for
 *                          this process. Unlike ons_init() and
 *                          ons_init_woraclehome(), ons_init_wconfig() does
 *                          not support nested calls. If ONS_ALREADYSTARTED
 *                          is returned no corresponding call to ons_shutdown()
 *                          is required to decrement a "nested" index.
 *
 * @see ons_get_global_ctx()
 * @see ons_init_wconfig_ctx()
 * @see ons_init_wconfig_common_ctx()
 * @see ons_init()
 * @see ons_init_woraclehome()
 * @see ons_shutdown()
 * @see ons_shutdown_wtimeout()
 * @see ons_nodelist
 */
int ons_init_wconfig(const char* config);

/**
 * Returns the global client context created by ons_init(),
 * ons_init_woraclehome(), or ons_init_wconfig()
 *
 * @return The ONS global client context pointer, or NULL if it does not exist
 *
 * @see ons_init()
 * @see ons_init_woraclehome()
 * @see ons_init_wconfig()
 */
const void* ons_get_global_ctx(void);

/**
 * Performs the setup required for a specific ONS client context that exists
 * and operates independent from any other ONS client context (including the
 * global context); multiple client contexts can be initialized, although it
 * is expected that the set of ONS servers specified by each is completely
 * unique. Initiates connections to the configured lists of ONS servers,
 * although it does not block waiting for the server connections to complete.
 * <p>
 * The caller must use the _ctx versions of the ONS APIs listed below to
 * shutdown the context, create subscribers or publisher, or get the
 * context's node-lists.
 * </p>
 *
 * @param[in] config A string specifying the configuration of the client.
 * See ons_init_wconfig() for a complete description.
 * @param[out] status The status of the initialization. See the
 * ons_init_wconfig() return values for details.
 *
 * @return The ONS client context pointer, or NULL on error.
 *
 * @see ons_init_wconfig()
 * @see ons_init_woraclehome_ctx()
 * @see ons_init_wconfig_common_ctx()
 * @see ons_shutdown_wtimeout_ctx()
 * @see ons_subscriber_create_wtimeout_ctx()
 * @see ons_subscriber_create_async_ctx()
 * @see ons_publisher_create_ctx()
 * @see ons_nodelists_ctx()
 * @see ons_nodelist
 */
const void* ons_init_wconfig_ctx(const char* config, int* status);

/**
 * Behaves the same as ons_init_wconfig_ctx(), but the context created is
 * common for all calls to ons_init_wconfig_common_ctx() with the exact same
 * <b>config</b> string.
 * <p>
 * The context pointer returned by ons_init_wconfig_common_ctx() will be the
 * same as returned by previous calls with the same <b>config</b> string,
 * unless previous matching common context was shutdown for the final
 * reference.
 * </p>
 * <p>
 * ons_shutdown_wtimeout_ctx() must be called for each successful call to
 * ons_init_wconfig_common_ctx(). ONS will track the references to the
 * common context and only perform the shutdown when the last reference
 * is shutdown.
 * </p>
 * <p>
 * Subscribers that have the same subscription string and other parameters
 * will also be managed as common, as will any registered call backs for
 * these subscribers.
 * </p>
 * <p>
 * The caller must use the _ctx versions of the ONS APIs listed below to
 * shutdown the context, create subscribers or publisher, or get the
 * context's node-lists.
 * </p>
 * @param[in] config A string specifying the configuration of the client.
 * See ons_init_wconfig() for a complete description.
 * @param[out] status The status of the initialization. See the
 * ons_init_wconfig() return values for details.
 *
 * @return The common ONS client context pointer, or NULL on error.
 *
 * @see ons_init_wconfig()
 * @see ons_init_woraclehome_ctx()
 * @see ons_init_wconfig_ctx()
 * @see ons_shutdown_wtimeout_ctx()
 * @see ons_subscriber_create_wtimeout_ctx()
 * @see ons_subscriber_create_async_ctx()
 * @see ons_publisher_create_ctx()
 * @see ons_nodelists_ctx()
 * @see ons_nodelist
 */
const void* ons_init_wconfig_common_ctx(const char* config, int* status);

/**
 * Shuts down the ONS client, closing the sockets to the servers and
 * releasing the active client resources. The ONS client uses a reference
 * count to track the number of times ons_init() or ons_init_woraclehome()
 * is called. This reference count is decremented with each call to
 * ons_shutdown(), ons_shutdown_wtimeout() or ons_shutdown_nowait(), and the
 * actual shut down of the ONS client only occurs when the reference count
 * reaches 0.
 *
 * This nesting of init and shutdown does not apply to ons_init_wconfig();
 *
 * Any active publishers, subscribers or rpcs should be destroyed before
 * shutting down the ONS client.
 *
 * This call will wait for all queued notifications to be delivered to each
 * list of servers before returning. Warning: this call can block forever.
 *
 * @see ons_init()
 * @see ons_init_woraclehome()
 * @see ons_init_wconfig()
 * @see ons_shutdown_wtimeout()
 * @see ons_shutdown_nowait()
 */
void ons_shutdown(void);

/**
 * Shuts down the ONS client, closing the sockets to the servers and
 * releasing the active client resources. The ONS client uses a reference
 * count to track the number of times ons_init() or ons_init_woraclehome()
 * is called. This reference count is decremented with each call to
 * ons_shutdown(), ons_shutdown_wtimeout() or ons_shutdown_nowait(), and the
 * actual shut down of the ONS client only occurs when the reference count
 * reaches 0.
 *
 * This nesting of init and shutdown does not apply to ons_init_wconfig();
 *
 * Any active publishers, subscribers or rpcs should be destroyed before
 * shutting down the ONS client.
 *
 * This call will wait up to timeout seconds for all queued notifications
 * to be delivered to each list of servers before returning. At the end of
 * the timeout any undelivered notifications will be discarded.
 *
 * @param[in]   timeout The timeout value in seconds.
 *
 * @see ons_init()
 * @see ons_init_woraclehome()
 * @see ons_init_wconfig()
 * @see ons_shutdown_wtimeout()
 * @see ons_shutdown_nowait()
 */
void ons_shutdown_wtimeout(int timeout);

/**
 * Shuts down the given ONS client context, closing all publishers and
 * subscribers.
 *
 * Any active publishers, subscribers or rpcs should be destroyed before
 * shutting down the ONS client.
 *
 * This call will wait up to timeout seconds for all queued notifications
 * to be delivered to each list of servers before returning. At the end of
 * the timeout any undelivered notifications will be discarded.
 *
 * @param[in] context The ONS context returned by ons_init_wconfig_ctx() or
 *                    ons_init_wconfig_common_ctx().
 * @param[in] timeout The timeout value in seconds.
 *
 * @see ons_init_wconfig_ctx()
 * @see ons_init_wconfig_common_ctx()
 */
void ons_shutdown_wtimeout_ctx(const void* context, int timeout);

/**
 * Shuts down the ONS client, closing the sockets to the servers and
 * releasing the active client resources. The ONS client uses a reference
 * count to track the number of times ons_init() or ons_init_woraclehome()
 * is called. This reference count is decremented with each call to
 * ons_shutdown(), ons_shutdown_wtimeout() or ons_shutdown_nowait(), and the
 * actual shut down of the ONS client only occurs when the reference count
 * reaches 0.
 *
 * This nesting of init and shutdown does not apply to ons_init_wconfig();
 *
 * Any active publishers, subscribers or rpcs should be destroyed before
 * shutting down the ONS client.
 *
 * This call will not wait for queued notifications to be delivered before
 * returning (all undelivered notifications will be discarded).
 *
 * @see ons_init()
 * @see ons_init_woraclehome()
 * @see ons_init_wconfig()
 * @see ons_shutdown_wtimeout()
 * @see ons_shutdown_nowait()
 */
void ons_shutdown_nowait(void);

/**
 * Provides the id values from the ONS server to which the client
 * has connected. If the client has not yet connected to the ONS
 * server, this function blocks until the connection is established
 * or the specified timeout has been reached.
 *
 * @deprecated  This information was only meaningful for the webtier.
 *
 * @param[out]  clusterId Deprecated entity: value is undefined.
 * @param[out]  clusterName Deprecated entity: value is undefined.
 * @param[out]  instanceId Deprecated entity: value is instanceName.
 * @param[out]  instanceName The name of the instance.
 * @param[in]   timeout The timeout value in seconds.
 *
 * @return One of the following codes:
 *   -  0 - Requested ids have been set.
 *   - -1 - Connection to ONS server was not established within the
 *          configured timeout value.
 */
int ons_get_ids_wtimeout(const char** clusterId,
      const char** clusterName, const char** instanceId,
      const char** instanceName, int timeout);

/**
 * Queries the global number of unique connections managed by the ONS client.
 *
 * The ONS server maintains a cache of connections based upon all configured
 * client contexts within the same process, and any configured connection
 * with the same port and SSL configuration will share a single connection
 * entity (socket, receive thread and other IO resources).
 *
 * Note that this is not necessarily the same as the number of active
 * connections the ONS client has with servers: that value may be less
 * than the returned value depending upon configured maxconnections
 * values and server status.
 *
 * @return The number of unique connections managed by the ONS client within
 *         the calling process.
 */
int ons_get_connections(void);

/**
 * Returns if the global ONS client context has been configured with SSL
 *
 * This call only applies to the global context. See ons_is_ssl_enabled_ctx()
 * for the per context version.
 *
 * @see ons_init_wconfig()
 * @see ons_is_ssl_enabled_ctx()
 *
 * @return One of the following codes:
 *   -  1 - SSL is enabled.
 *   -  0 - SSL is not enabled.
 *   - -1 - The global context is not initialized.
 */
int ons_is_ssl_enabled(void);

/**
 * Returns if the specified ONS client context has been configured with SSL
 *
 * @param[in]   ctx The context pointer returned by ons_init_wconfig_ctx()
 *                  or ons_init_wconfig_common_ctx(). It may be NULL, but it
 *                  must otherwise be a valid context that has not yet been
 *                  shut down (passing in an invalid or shut down context
 *                  pointer will result in invalid memory references).
 *
 * @see ons_init_wconfig_ctx()
 * @see ons_init_wconfig_common_ctx()
 *
 * @return One of the following codes:
 *   -  1 - SSL is enabled.
 *   -  0 - SSL is not enabled.
 *   - -1 - Invalid context pointer.
 */
int ons_is_ssl_enabled_ctx(const void* ctx);

/* @} */

/**
 * @defgroup NodeList  Node lists and connections
 * @ingroup ONS
 *
 * @{
 */

/**
 * The entity that manages a collection of ONS server connections. Each list
 * represents a unique topology of ONS servers.
 *
 * @see ons_nodelists()
 * @see ons_nodelist_release()
 * @see ons_nodelist_next()
 * @see ons_nodelist_id()
 * @see ons_nodelist_active()
 * @see ons_nodelist_concurrency()
 * @see ons_nodelist_failover_list()
 * @see ons_nodelist_connections()
 * @see ons_connection
 */
typedef struct _ons_nodelist ons_nodelist;

/**
 * The entity that manages a single end point for a connection to an
 * ONS server. A connection structure represents a configured host:port
 * pair that may or may not be actually connected to its designated
 * server at any given point in time.
 *
 * @see ons_nodelist_connections()
 * @see ons_connection_release()
 * @see ons_connection_next()
 * @see ons_connection_id()
 * @see ons_connection_host()
 * @see ons_connection_port()
 * @see ons_connection_status()
 * @see ons_connection_ping()
 * @see ons_nodelist
 * @see ons_connection_state
 */
typedef struct _ons_connection ons_connection;

/**
 * Values for the state of each connection.
 */
typedef enum
{
  /** The connection is idle */
  OnsConnectionStateIdle = 0,
  /** The connection is connected to the server */
  OnsConnectionStateConnected,
 /** The connection is attempting to establish a connection with the server */
  OnsConnectionStateConnecting,
  /** The connection is shut down */
  OnsConnectionStateShutdown
} ons_connection_state;

/**
 * Provides the head of the list of node-lists.
 *
 * @return The head of the list of node-list or NULL if the ONS is not
 *         initialized and ready. The non-NULL pointer must be released via
 *         a call to either ons_nodelist_next() or ons_nodelist_release().
 *
 * @see ons_nodelist_next()
 * @see ons_nodelist_release()
 */
const ons_nodelist* ons_nodelists(void);

/**
 * Provides the head of the list of node-lists for the given ONS client
 * context.
 *
 * @param[in] context The ONS context returned by ons_init_wconfig_ctx() or
 *                    ons_init_wconfig_common_ctx().
 *
 * @return The head of the list of node-list or NULL if the ONS is not
 *         initialized and ready. The non-NULL pointer must be released via
 *         a call to either ons_nodelist_next() or ons_nodelist_release().
 *
 * @see ons_init_wconfig_ctx()
 * @see ons_init_wconfig_common_ctx()
 * @see ons_nodelist_next()
 * @see ons_nodelist_release()
 */
const ons_nodelist* ons_nodelists_ctx(const void* context);

/**
 * Releases the given node-list structure.
 *
 * @param[in]   nodeList A node-list element. This element should not be
 *                       referenced after this call is made.
 */
void ons_nodelist_release(const ons_nodelist* nodeList);

/**
 * Provides the next node-list in the list.
 *
 * @param[in]   nodeList A node-list element. This element will be released
 *                       upon return and should not be referenced again.
 *
 * @return The next element of the list of node-list list or NULL if
 *         the argument was the last element of the list. A non-NULL pointer
 *         must be released via a subsequent call to ons_nodelist_next() or
 *         a call to ons_nodelist_release().
 *
 * @see ons_nodelist_release()
 */
const ons_nodelist* ons_nodelist_next(const ons_nodelist* nodeList);

/**
 * Provides the id string for a node-list.
 *
 * @param[in]   nodeList A node-list element.
 *
 * @return The id string for the node-list. This string should not be
 *         referenced after the node-list has been released.
 */
const char* ons_nodelist_id(const ons_nodelist* nodeList);

/**
 * Indicates if the node-list is active or inactive
 *
 * @param[in]   nodeList A node-list element.
 *
 * @return 1 if the list is active; otherwise 0.
 */
int ons_nodelist_active(const ons_nodelist* nodeList);

/**
 * Provides the node-list connection concurrency value.
 *
 * @param[in]   nodeList A node-list element.
 *
 * @return The number of concurrent connections the node-list will attempt
 *         to maintain.
 */
int ons_nodelist_concurrency(const ons_nodelist* nodeList);

/**
 * Provides the fail over node-node list for this node list.
 *
 * @param[in]   nodeList A node-list element.
 *
 * @return     The node-list fail over list: this may be null. If the return
 *             list is not null then it represents either the list to which
 *             the calling list has failed over (if the calling list is
 *             active) or the list which has failed over to the calling list
 *             (if the calling list is not active).
 *
 *             If the return value is not null, then the caller <b>must</b>
 *             release the returned node-list via ons_nodelist_release()
 *             when done referencing it.
 *
 * @see ons_nodelist_release()
 */
const ons_nodelist* ons_nodelist_failover_list(const ons_nodelist* nodeList);

/**
 * Provides the head of the node-list's connection list.
 *
 * @param[in]   nodeList A node-list element.
 *
 * @return The head of the node-list's connection list. The pointer must be
 *         released via a call to either ons_connection_next() or
 *         ons_connection_release().
 *
 * @see ons_connection_next()
 * @see ons_connection_release()
 */
const ons_connection* ons_nodelist_connections(const ons_nodelist* nodeList);

/**
 * Releases the given connection structure.
 *
 * @param[in]   connection A connectionelement. This element should not be
 *                         referenced after this call is made.
 */
void ons_connection_release(const ons_connection* connection);

/**
 * Provides the next connection in the list.
 *
 * @param[in]   connection A connection element. This element will be released
 *                         upon return and should not be referenced again.
 *
 * @return The next element of the connection list or NULL if the argument was
 *         the last element of the list. A non-NULL pointer must be released
 *         via a subsequent call to ons_connection_next() or a call to
 *         ons_connection_release().
 *
 * @see ons_connection_release()
 */
const ons_connection* ons_connection_next(const ons_connection* connection);

/**
 * Provides the id string for a connection.
 *
 * @param[in]   connection A connection element.
 *
 * @return The id string for the connection. This string should not be
 *         referenced after the connection has been released.
 */
const char* ons_connection_id(const ons_connection* connection);

/**
 * Provides the host string for a connection.
 *
 * @param[in]   connection A connection element.
 *
 * @return The host string for the connection. This string should not be
 *         referenced after the connection has been released.
 */
const char* ons_connection_host(const ons_connection* connection);

/**
 * Provides the port value for a connection.
 *
 * @param[in]   connection A connection element.
 *
 * @return The port value for the connection.
 */
int ons_connection_port(const ons_connection* connection);

/**
 * Provides the state of a connection.
 *
 * @param[in]   connection A connection element.
 *
 * @return The state of the connection.
 *
 * @see ons_connection_status_string()
 */
ons_connection_state ons_connection_status(const ons_connection* connection);

/**
 * Provides A string for the given connection state value
 *
 * @param[in]   state The ons_connection_state returned from
 *                    ons_connection_status().
 *
 * @return A string representation of state.
 *
 * @see ons_connection_status()
 */
const char* ons_connection_status_string(ons_connection_state state);

/**
 * Ping the connection's server. The ping is performed independently of the
 * connection's current state with the server.
 *
 * @param[in]   connection A connection element.
 *
 * @return 0 if the ping succeeded, otherwise -1.
 */
int ons_connection_ping(const ons_connection* connection);

/* @} */

/**
 * @defgroup Notification  Notification creation and access
 * @ingroup ONS
 *
 * @{
 */

/**
 * The basic message entity used for sending and receiving notifications.
 * <p>
 * The ONS server silently discards any notification exceeding the following
 * size limitations:
 * <ul>
 * <li> The total notification header size must be less than 1 megabyte
 *      (1048576) -- this includes all header field names and values
 *      (such as notification type, components, and nodes), property names
 *      and values, plus any internal formatting required to represent
 *      the notification as an HTTP message. The overhead of an ONS
 *      header (without user supplied values or properties) is about
 *      1024 bytes, leaving approximately 1047552 bytes for user supplied
 *      header values as well as property names and values.</li>
 * <li> The maximum body length is 8 megabytes (8380416).</li>
 * </ul>
 * </p>
 * <p>
 * In addition the ONS client has its own restrictions on notification
 * header name and value lengths (including properties). No given header
 * field name and value (or property name and value) can exceed 16380 bytes
 * in length, nor can they include a colon (property name restriction only),
 * a newline or carriage return character. The ONS client will reject any
 * properties that exceed this limit, and the ONS server will close the
 * connection to any client that exceeds this limit.
 * </p>
 *
 * @see ons_notification_create()
 * @see ons_subscriber_receive()
 */
typedef struct _ons_message ons_notification;

/**
 * Name value pair structure.
 */
typedef struct _ons_namevalue
{
  const char *name;
  const char *value;
} ons_namevalue;

/**
 * Creates a notification that can be published.
 *
 * The type, components, and nodes arguments are all part of the
 * notification meta-data that is to be attached to the notification.
 * The body and bodyLen arguments define the payload for the notification.
 *
 * @param[in] type         A hierarchical string similar in structure to
 *                         the component name, such as "ias/webcache/startup".
 *                         Must be a non-NULL string.
 * @param[in] components   Components affected by this notification.
 *                         Must be a non-NULL string, but may be an empty
 *                         string ("").
 * @param[in] nodes        Nodes affected by this notification.
 *                         Must be a non-NULL string, but may be an empty
 *                         string ("").
 * @param[in] body         Payload for the notification. Can contain any
 *                         arbitrary byte data, and no assumptions are made
 *                         about NULL terminating the array. May be NULL
 *                         if bodyLen is 0.
 * @param[in] bodyLen      The length of the body data. The ONS server will
 *                         silently reject any notification with a body length
 *                         greater than 8 megabytes (8380416).
 *
 * @see ons_publisher_publish()
 *
 * @return A new notification to be published.
 */
ons_notification *ons_notification_create(const char *type,
                    const char *components, const char *nodes,
                    const char *body, int bodyLen);

/**
 * Adds a body and length to a notification created by a call to
 * ons_notification_create() with a NULL value for body and 0 for bodyLen
 *
 * @param[in] n            A notification created by ons_notification_create()
 *
 * @param[in] body         Payload for the notification. Can contain any
 *                         arbitrary byte data, and no assumptions are made
 *                         about NULL terminating the array. May be NULL, but
 *                         this is a nop.
 * @param[in] bodylen      The length of the body data. The ONS server will
 *                         silently reject any notification with a body length
 *                         greater than 8 megabytes (8380416). A value of 0
 *                         is a nop.
 *
 * @see ons_notification_create()
 *
 * @return 0 if the body was added to the notification, 1 if the notification
 *         was created with an existing body (it was not replaced), and
 *         -1 for memory allocation error.
 */
int ons_notification_body_add(ons_notification *n, const char *body,
      int bodylen);

/**
 * Returns the type of the notification. The string returned is the string
 * specified for "type" when the notification was created.
 *
 * @param[in] n A valid notification.
 *
 * @return The notification type.
 */
const char *ons_notification_type(ons_notification *n);

/**
 * Returns the affected components for the notification. The string
 * returned is the string specified for "components" when the
 * notification was created.
 *
 * @param[in] n A valid notification.
 *
 * @return The affected components for the notification. May be NULL.
 */
const char *ons_notification_affectedComponents(ons_notification *n);

/**
 * Returns the affected nodes for the notification. The string
 * returned is the string specified for "nodes" when the
 * notification was created.
 *
 * @param[in] n A valid notification.
 *
 * @return The affected nodes for the notification. May be NULL.
 */
const char *ons_notification_affectedNodes(ons_notification *n);

/**
 * Returns the body of the notification. The character array returned is
 * the array specified for "body" when the notification was created, and
 * bodyLen is set to the body length specified when the notification was
 * created.
 *
 * @param[in] n A valid notification.
 * @param[out] bodyLen The length of the notification body.
 *
 * @return The body of the notification. May be NULL if NULL was specified
 *         for the body when the notification was created.
 */
const char *ons_notification_body(ons_notification *n, int *bodyLen);

/**
 * Returns the creation time (in seconds) of the notification. The
 * creation time is the time at which the notification was delivered
 * to the server.
 *
 * @param[in] n A valid notification.
 *
 * @return The creation time (in seconds) of the notification.
 */
long ons_notification_creationTime(ons_notification *n);

/**
 * Returns the delivery time (in seconds) of the notification.
 *
 * @param[in] n A valid notification.
 *
 * @return The delivery time (in seconds) of the notification.
 */
long ons_notification_deliveryTime(ons_notification *n);

/**
 * Returns the component of the notification's publisher -- this is the value
 * specified for the "component" argument to ons_publisher_create() or
 * ons_subscriber_create().
 *
 * @param[in] n A valid notification.
 *
 * @return The generating component of the notification. May be NULL.
 *
 * @see ons_publisher_create()
 * @see ons_subscriber_create()
 */
const char *ons_notification_generatingComponent(ons_notification *n);

/**
 * Returns the node of the notification's publisher -- this is system on
 * which the publisher was running.
 *
 * @deprecated  May be the host name for the server to which the notification
 * was delivered.
 *
 * @param[in] n A valid notification.
 *
 * @return The generating node of the notification. May be NULL.
 */
const char *ons_notification_generatingNode(ons_notification *n);

/**
 * Returns the process of the notification's publisher -- this is process in
 * which the publisher was running.
 *
 * @deprecated  This value is no longer supported.
 *
 * @param[in] n A valid notification.
 *
 * @return The generating process of the notification. May be NULL.
 */
const char *ons_notification_generatingProcess(ons_notification *n);

/**
 * Returns the notification origin stamp -- the stamp is assigned by the
 * ONS server to which the notification was directly published by the
 * client. The format of the origin stamp is not defined and may change
 * without notice (it is not intended for general use).
 *
 * @param[in] n A valid notification.
 *
 * @return The notification stamp.
 */
const char *ons_notification_stamp(ons_notification *n);

/**
 * Returns the value for the given property name in the notification.
 *
 * @param[in] n A valid notification.
 * @param[in] name A non-NULL property name.
 *
 * @return The value of the property, or NULL if the property does not
 *         exist in the notification.
 *
 * @see ons_notification_setProperty()
 */
const char *ons_notification_getProperty(ons_notification *n,
              const char *name);

/**
 * Returns an array of name/value elements for all properties in the
 * notification. The array (not the contents) must be freed by
 * the caller via the free() library call.
 *
 * @param[in] n A valid notification.
 * @param[out] numProps The number of properties in the returned array.
 *
 * @return An array of numProps ons_namevalue elements: must be freed
 *         by the caller via free(). May return NULL.
 *
 * @see ons_notification_setProperty()
 */
ons_namevalue *ons_notification_getAllProperties(ons_notification *n,
                 int *numProps);

/**
 * Returns the notification id assigned by the publisher.
 *
 * @param[in] n A valid notification.
 *
 * @return The notification id.
 */
const char *ons_notification_id(ons_notification *n);

/**
 * Returns the notification cluster name, an undefined string.
 *
 * @deprecated  This information was only meaningful for the webtier.
 *
 * @param[in] n A valid notification.
 *
 * @return The notification cluster name. May return NULL.
 */
const char *ons_notification_clusterName(ons_notification *n);

/**
 * Returns the notification instance name, and undefined string.
 *
 * @deprecated  This information was only meaningful for the webtier.
 *
 * @param[in] n A valid notification.
 *
 * @return The notification instance name. May return NULL.
 */
const char *ons_notification_instanceName(ons_notification *n);

/**
 * Returns the notification local only setting.
 *
 * @param[in] n A valid notification.
 *
 * @return 0 if the notification is not set local only, and non zero if it is.
 *
 * @see ons_notification_setLocalOnly()
 */
int ons_notification_localOnly(ons_notification *n);

/**
 * Returns the notification version, which is an internal ONS protocol
 * version identifier.
 *
 * @param[in] n A valid notification.
 *
 * @return The notification ONS protocol version.
 */
int ons_notification_version(ons_notification *n);

/**
 * Associate a name value pair with a notification.
 *
 * No property name or value combined may exceed 2044 bytes in
 * length or they will be rejected as invalid.
 *
 * @param[in] n A valid notification.
 * @param[in] name A property name, which must not be NULL and must not
 *            include a colon, newline or carriage return. The following
 *            names are reserved for ONS notification headers and should
 *            not be used for property names:<br>
 * <tt>origin</tt><br>
 * <tt>stamp</tt><br>
 * <tt>Version</tt><br>
 * <tt>eventType</tt><br>
 * <tt>affectedComponents</tt><br>
 * <tt>affectedNodes</tt><br>
 * <tt>generatingComponent</tt><br>
 * <tt>generatingProcess</tt><br>
 * <tt>generatingNode</tt><br>
 * <tt>eventId</tt><br>
 * <tt>creationTime</tt><br>
 * <tt>clusterId</tt><br>
 * <tt>clusterName</tt><br>
 * <tt>instanceId</tt><br>
 * <tt>instanceName</tt><br>
 * <tt>LocalOnly</tt><br>
 * <tt>ClusterOnly</tt><br>
 * <tt>numberOfProperties</tt><br>
 * @param[in] value A property value, which must not be NULL and must not
 *            include a newline or carriage return.
 *
 * @return 0 if the property was associated with the notification, -1 if
 * the property name or value is invalid, or -2 if a memory allocation
 * error was encountered.
 */
int ons_notification_setProperty(ons_notification *n,
      const char *name, const char *value);

/**
 * Sets the local only flag for the notification. This informs the
 * ONS server to which this notification is published to only
 * delivery the notification to other clients within the local
 * instance.
 *
 * @param[in] n A valid notification.
 */
void ons_notification_setLocalOnly(ons_notification *n);

/**
 * Prints the notification headers to stderr. If there is a body, it prints
 * the body length and attempts to print the body as a string. For debug
 * purposes only.
 *
 * @param[in] n A valid notification.
 */
void ons_notification_print(ons_notification *n);

/* @} */

/**
 * @defgroup Subscriber Subscriber creation and operation
 * @ingroup ONS
 *
 * @{
 */

/**
 * Callback types for ons_subscriber_registercallback() and
 * ons_rpcserver_setcbmode().
 */
typedef enum _ons_cb_type
{
  /** A persistent thread will service all CBs */
  ONS_ONECBTHREAD,
  /** A new thread will be created as needed to service the CBs */
  ONS_THREADPERCB
} ons_cb_type;

/*
 * Return values for ons_subscriber_status().
 */
/** Subscriber has connected to the server and is ready for use */
#define ONS_SUBSCRIBER_CONNECTED   0
/** Subscriber is waiting to connect to the server */
#define ONS_SUBSCRIBER_WAITING     1
/** Subscriber initialization has failed */
#define ONS_SUBSCRIBER_FAILED      2

/** Subscription that will not match any notifications */
#define ONS_SUBSCRIPTION_NOMATCH   "!"

/**
 * <p>
 * Each subscriber is created with a corresponding subscription string. The
 * ONS server compares published notifications against each subscriber's
 * subscription string to determine if the notification should be delivered
 * to the subscriber.
 * </p>
 * <p>
 * <b>The Subscription</b>
 * </p>
 * <p>
 * The subscription string is composed of one or more <i>comparison</i>
 * statements, each logically related to another with the boolean operators
 * <tt>'|'</tt> for an <tt>OR</tt> relationship or <tt>'&'</tt> for an
 * <tt>AND</tt> relationship. Parentheses are used to group these
 * <i>comparison</i> statements, and the <tt>'!'</tt> operator placed before
 * an opening parenthesis negates the evaluated value within.
 * </p>
 * <p>
 * Each individual <i>comparison</i> statement must be enclosed within double
 * quotes (<tt>'"'</tt>), and can take one of two basic forms:
 * <i>"pattern"</i> or <i>"name=value"</i>.
 * </p>
 * <p>
 * A <i>"pattern"</i> is a simple string match of the notification header:
 * if the specified <i>"pattern"</i> occurs anywhere in a notification's
 * header, then the comparison statement evaluates true. The most basic
 * pattern match is an empty string (not NULL) which matches all notifications.
 * </p>
 * <p>
 * The <i>"name=value"</i> format compares the ONS notification header or
 * property name with the <i>name</i> against the specified <i>value</i>,
 * and if the values match, then the comparison statement evaluates true.
 * If the specified header or property <i>name</i> does not exist in
 * the notification the comparison statement evaluates false.
 * </p>
 * <p>
 * A comparison statement will be interpreted as a <i>case insensitive</i>
 * when a percent character (<tt>'%'</tt>) is placed before the opening quote.
 * Note that for <i>"name=value"</i> comparison statements, only the
 * <i>value</i> is treated as case insensitive with this option: <b>the
 * <i>name</i> lookup will always be case sensitive</b>.
 * </p>
 * <p>
 * A comparison statement will be interpreted as a <i>regular expression</i>
 * when a dollar sign character (<tt>'$'</tt>) is placed before the opening
 * quote. Standard POSIX regular expressions are supported. To specify a
 * <i>regular expression</i> that is also <i>case insensitive</i>, place
 * the dollar sign and percent sign together and in that order (<tt>"$%"</tt>)
 * before the opening quote.
 * </p>
 * <p>
 * A special case subscription string composed of only the exclamation point
 * character (<tt>'!'</tt>) signifies that the subscription will not match any
 * notifications.
 * </p>
 * <p>
 * Examples:
 * </p>
 * <p>
 * The examples represent the exact strings passed to the ONS server, and
 * may require additional syntax in any given programming or scripting
 * language to escape or represent some of the characters shown in the
 * examples. For example, in C or Java each double quote character
 * (<tt>'"'</tt>) in a literal subscription string will need to be escaped
 * with a backslash (<tt>'\'</tt>) character.
 * </p>
 * <p>
 * Match all PROC_READY notifications:<br>
 * <tt>"eventType=IAS/PM/PROC_READY"</tt>
 * </p>
 * <p>
 * Match PROC_READY notifications for OHS (or generic Apache):<br>
 * <tt>("eventType=IAS/PM/PROC_READY")&("generatingComponent=IAS/APACHE")</tt>
 * </p>
 * <p>
 * Match PROC_READY notifications <i>except</i> for OHS:<br>
 * <tt>("eventType=IAS/PM/PROC_READY")&!("generatingComponent=IAS/APACHE")</tt>
 * </p>
 * <p>
 * Match PROC_READY notifications for OHS and WebCache:<br>
 * <tt>("eventType=IAS/PM/PROC_READY")&(("generatingComponent=IAS/APACHE")|("generatingComponent=IAS/WEBCACHE"))</tt>
 * </p>
 * <p>
 * Match any IAS/PM related notification for OHS, but only if there is
 * body data (note the use of both the regular expression and case
 * insensitive flags):<br>
 * <tt>($%%"eventtype: ias/pm/.*")&(%%"generatingComponent=ias/apache")&!("Content-Length=0")</tt>
 * </p>
 */
typedef struct _ons_subscriber ons_subscriber;

/*
 * Note, for the last example above "%%" is used rather than '%' to escape
 * the doxygen consumption of a single '%': the correct syntax for the
 * subscription is:
 * ($%"eventtype: ias/pm/.*")&(%"generatingComponent=ias/apache")&!("Content-Length=0")
 */

/**
 * Each subscriber has an associated publisher used for sending the
 * subscription to the ONS server. The publisher inherits the
 * subscriber's component id, and users can use the subscriber's
 * publisher to publish their own notifications, or create their own
 * publisher.
 */
typedef struct _ons_publisher ons_publisher;

/**
 * Creates a subscriber based upon the provided subscription and
 * component id.
 *
 * This call will block until the ONS client is able to connect to the
 * ONS server if it has not already established a connection to the server.
 * All subscription parsing and management is performed by the server.
 *
 * @param[in] subscription A valid, non-NULL subscription string.
 * @param[in] component A non-NULL component id string.
 * @param[out] message A pointer to an error string that, if not NULL, is set
 *             if the subscriber creation failed. The message describes the
 *             problem with the subscription, usage errors, or other issues.
 *             If the value of *message is not NULL it must be freed.
 *
 * @return The subscriber. The subscriber resources should be released via
 *         ons_subscriber_close().
 *
 * @see ons_subscriber_create_wtimeout()
 * @see ons_subscriber_create_async()
 * @see ons_subscriber_close()
 */
ons_subscriber *ons_subscriber_create(const char *subscription,
                  const char *component, char **message);

/**
 * Creates a subscriber based upon the provided subscription and
 * component id.
 *
 * This call will block until the ONS client is able to connect to the
 * ONS server if it has not already established a connection to the server,
 * or until the specified timeout occurs.
 *
 * @param[in] subscription A valid, non-NULL subscription string.
 * @param[in] component A non-NULL component id string.
 * @param[out] message A pointer to an error string that, if not NULL, is set
 *             if the subscriber creation failed. The message describes the
 *             problem with the subscription, usage errors, or other issues.
 *             If the value of *message is not NULL it must be freed.
 * @param[in] timeout The timeout value in seconds.
 *
 * @return The subscriber. The subscriber resources should be released via
 *         ons_subscriber_close().
 *
 * @see ons_subscriber_create()
 * @see ons_subscriber_create_async()
 * @see ons_subscriber_close()
 */
ons_subscriber *ons_subscriber_create_wtimeout(const char *subscription,
                  const char *component, char **message, long timeout);

/**
 * Creates a subscriber based upon the provided subscription and
 * component id for the given ONS client context.
 *
 * This call will block until the ONS client is able to connect to the
 * ONS server if it has not already established a connection to the server,
 * or until the specified timeout occurs.
 *
 * @param[in] context The ONS context returned by ons_init_wconfig_ctx() or
 *                    ons_init_wconfig_common_ctx().
 * @param[in] subscription A valid, non-NULL subscription string.
 * @param[in] component A non-NULL component id string.
 * @param[out] message A pointer to an error string that, if not NULL, is set
 *             if the subscriber creation failed. The message describes the
 *             problem with the subscription, usage errors, or other issues.
 *             If the value of *message is not NULL it must be freed.
 * @param[in] timeout The timeout value in seconds.
 *
 * @return The subscriber. The subscriber resources should be released via
 *         ons_subscriber_close().
 *
 * @see ons_init_wconfig_ctx()
 * @see ons_init_wconfig_common_ctx()
 * @see ons_subscriber_create_async_ctx()
 * @see ons_subscriber_close()
 */
ons_subscriber* ons_subscriber_create_wtimeout_ctx(const void* context,
                  const char* subscription, const char* component,
                  char** message, long timeout);

/**
 * Creates a subscriber based upon the provided subscription and
 * component id.
 *
 * This call will return a subscriber without blocking, however the subscriber
 * cannot be used until a subsequent call to ons_subscriber_status() returns
 * ONS_SUBSCRIBER_CONNECTED.
 *
 * @param[in] subscription A valid, non-NULL subscription string.
 * @param[in] component A non-NULL component id string.
 *
 * @return The subscriber. The subscriber resources should be released via
 *         ons_subscriber_close().
 *
 * @see ons_subscriber_create()
 * @see ons_subscriber_create_wtimeout()
 * @see ons_subscriber_status()
 * @see ons_subscriber_close()
 */
ons_subscriber *ons_subscriber_create_async(const char *subscription,
                  const char *component);

/**
 * Creates a subscriber based upon the provided subscription and
 * component id for the given ONS client context.
 *
 * This call will return a subscriber without blocking, however the subscriber
 * cannot be used until a subsequent call to ons_subscriber_status() returns
 * ONS_SUBSCRIBER_CONNECTED.
 *
 * @param[in] context The ONS context returned by ons_init_wconfig_ctx() or
 *                    ons_init_wconfig_common_ctx().
 * @param[in] subscription A valid, non-NULL subscription string.
 * @param[in] component A non-NULL component id string.
 *
 * @return The subscriber. The subscriber resources should be released via
 *         ons_subscriber_close().
 *
 * @see ons_init_wconfig_ctx()
 * @see ons_init_wconfig_common_ctx()
 * @see ons_subscriber_create_wtimeout_ctx()
 * @see ons_subscriber_status()
 * @see ons_subscriber_close()
 */
ons_subscriber* ons_subscriber_create_async_ctx(const void* context,
                  const char* subscription, const char* component);

/**
 * Returns the status of an asynchronously created subscriber.
 *
 * @param[in] s The subscriber returned from ons_subscriber_create_async().
 * @param[out] message A pointer to an error string that is set if the
 *             subscriber creation failed because of an invalid subscription.
 *             The message describes the problem with the subscription. Note
 *             that the message pointer is only set on subscription errors,
 *             and so users should set the message pointer to NULL before
 *             calling and check the value against NULL before dereferencing.
 *
 * @return The subscriber status:
 *
 * - ONS_SUBSCRIBER_CONNECTED - if the subscriber has connected to the server
 *   and is ready to use,
 * - ONS_SUBSCRIBER_WAITING - if the subscriber has not yet connected to the
 *   server, and
 * - ONS_SUBSCRIBER_FAILED - if the the subscription string was invalid.
 *
 * @see ons_subscriber_create_async()
 */
int ons_subscriber_status(ons_subscriber *s, char **message);

/**
 * Closes the subscriber and releases its resources.
 *
 * @param[in] s A valid subscriber.
 *
 * @see ons_subscriber_create()
 * @see ons_subscriber_create_wtimeout()
 * @see ons_subscriber_create_async()
 */
void ons_subscriber_close(ons_subscriber *s);

/**
 * Create and register as subscriber for a disconnect notification -- the
 * subscription is set to match no notifications.
 *
 * The registerId and disconnectId strings must be composed of printable ASCII
 * characters except for the asterisk ('@'), the semicolon (';') and any white
 * space other than a space (' '). The strings must be between 1 and 1024 bytes
 * in length.
 *
 * @param[in]  onsCtx The ONS client context.
 *
 * @param[in]  registerId Subscriber's registration id string.
 *
 * @param[in]  disconnectId Subscriber's disconnect id string.
 *
 * @param[in]  timeOut Time out in seconds for subscriber creation
 *                     and registration. Use 0 to block with no time out.
 *
 * @param[out]  error 0 for success; otherwise the error code for this
 *                    operation (from onserror.h):
 * <ul>
 * <li>OnsErrorSubscriberCreate Subscriber creation failed</li>
 * <li>OnsErrorNotificationCreate Register notification creation failed</li>
 * <li>OnsErrorNotificationReceive Failed to receive registration status</li>
 * <li>OnsErrorSubscriberRegister Subscriber registration failed</li>
 * <li>OnsErrorServerSupport ONS server does not support this feature</li>
 * </ul>
 *
 * @return A pointer to an ONS scubscriber on success; otherwise NULL.
 *
 * @see ons_subscriber_close_disconnect()
 */
ons_subscriber* ons_subscriber_create_disconnect_ctx(const void* onsCtx,
                  const char* registerId, const char* disconnectId,
                  long timeOut, int* error);

/**
 * Unregister and close a subscriber
 *
 * @param[in]  subscriber The ONS subscriber to unregister
 *
 * @param[in]  registerId Subscriber's registration id string.
 *
 * @param[in]  timeOut Time out in seconds for subscriber unregistration
 *                     verification. Use 0 to block with no time out.
 *
 * @param[out]  error 0 for success; otherwise the error code for this
 *                    operation (from onserror.h):
 * <ul>
 * <li>OnsErrorNotificationCreate Unregister notification creation failed</li>
 * <li>OnsErrorNotificationReceive Failed to receive registration status</li>
 * </ul>
 *
 * @return 0 if the unregistration was verified, otherwise non zero.
 *
 * @see ons_subscriber_create_disconnect_ctx()
 */
int ons_subscriber_close_disconnect(ons_subscriber* subscriber,
      const char* registerId, long timeOut, int* error);

/**
 * Returns the component string used to create the subscriber.
 *
 * @param[in] s A valid subscriber.
 *
 * @return The subscriber component id.
 */
const char *ons_subscriber_component(ons_subscriber *s);

/**
 * Returns the subscription string used to create the subscriber.
 *
 * @param[in] s A valid subscriber.
 *
 * @return The subscriber subscription string.
 */
const char *ons_subscriber_subscription(ons_subscriber *s);

/**
 * Returns the publisher for the subscriber.
 *
 * Each subscriber has its own publisher, which has the same component string
 * used to create the subscriber. This publisher may be used to publish any
 * notification.
 *
 * @param[in] s A valid subscriber.
 *
 * @return The subscriber's publisher.
 */
ons_publisher *ons_subscriber_getPublisher(ons_subscriber *s);

/**
 * Returns the subscriber id, which is an integer value used by the client
 * and server.
 *
 * @param[in] s A valid subscriber.
 *
 * @return The subscriber id.
 */
int ons_subscriber_id(ons_subscriber *s);

/**
 * Returns a notification for the subscriber.
 *
 * @param[in] s A valid subscriber. If the subscriber was created via
 *              ons_subscriber_create_async(), then its status must be
 *              ONS_SUBSCRIBER_CONNECTED or the call will immediately
 *              return NULL.
 * @param[in] blocking A flag that when zero specifies a non-blocking call
 *                     or non-zero specifies a blocking call.
 * @param[in] waittime If blocking is non-zero, this specifies the number
 *                     of milliseconds to wait for the next notification. A
 *                     value <= 0 specifies to block indefinitely until
 *                     a notification arrives.
 *
 * @return A notification delivered to the subscriber, or NULL if no
 *         notification was available and the call was non-blocking,
 *         or no notification was available within the specified timeout,
 *         or the subscriber was closed or ONS client shut down. If a
 *         non-NULL notification is returned, it must be released via
 *         a call to ons_subscriber_relinquish().
 *
 * @see ons_subscriber_relinquish()
 */
ons_notification *ons_subscriber_receive(ons_subscriber *s, int blocking,
                    int waittime);

/**
 * Releases the notification resources.
 *
 * @param[in] s A valid subscriber or NULL.
 * @param[in] n A notification returned by ons_subscriber_receive() or
 *              passed into the user call back function.
 *
 * @see ons_subscriber_receive()
 * @see ons_subscriber_registercallback()
 * @see ons_subscriber_registercallback_common()
 */
void ons_subscriber_relinquish(ons_subscriber *s, ons_notification *n);

/**
 * Registers a call back function for handling notifications for the
 * subscriber.
 *
 * Only one call back may be registered at a time for each subscriber.
 * Users should not attempt to call ons_subscriber_receive() for a
 * subscriber with a registered call back.
 *
 * ons_subscriber_registercallback() may called multiple times for the same
 * subscriber if the subscriber was created for an ONS client context created
 * via ons_init_wconfig_common_ctx() as long as all of the parameters are the
 * same. See ons_subscriber_registercallback_common().
 *
 * @param[in] s A valid subscriber. If the subscriber was created via
 *              ons_subscriber_create_async(), then its status must be
 *              ONS_SUBSCRIBER_CONNECTED.
 * @param[in] cb The call back function that will be called for each
 *               notification received. The call back function <b>must</b>
 *               relinquish each notification via ons_subscriber_relinquish()
 *               before returning. The arguments are:
 * <ul>
 * <li>n The notification for cb to service, which must be relinquished
 *       before cb returns via a call to ons_subscriber_relinquish()</li>
 * <li>ctx The context passed to ons_subscriber_registercallback() as ctx</li>
 * </ul>
 * @param[in] ctx A user provided context pointer (it can be NULL) which
 *                is passed in as the second argument to the user
 *                call back function.
 * @param[in] mode The call back mode, which must be either
 * <ul>
 * <li>ONS_ONECBTHREAD which uses the same thread to make all call back
 *                     function calls (the calls will be serialized),</li>
 * <li>ONS_THREADPERCB which creates a new thread as needed for the
 *                     callbacks.</li>
 * </ul>
 *
 * @return 0 if the call back was registered or -1 if not.
 *
 * @see ons_subscriber_relinquish()
 * @see ons_subscriber_cancelcallback()
 */
int ons_subscriber_registercallback(ons_subscriber *s,
      void (*cb)(ons_notification *n, void *ctx),
      void *ctx, ons_cb_type mode);

/**
 * Return values for ons_subscriber_registercallback_common()
 */
typedef enum _ons_cb_register_status
{
  /** The subscriber call back was successfully registered for the first time */
  ONS_REGISTER_CB_FIRST = 0,
  /** The subscriber call back was successfully registered again */
  ONS_REGISTER_CB_AGAIN,
  /** The subscriber call back register failed because ctxInit returned NULL */
  ONS_REGISTER_CB_FAILED_INIT,
  /** The subscriber call back register failed because it is already registered
      with a different set of parameters */
  ONS_REGISTER_CB_FAILED_BUSY,
  /** The subscriber call back register failed because of invalid arguments */
  ONS_REGISTER_CB_FAILED_ARGS,
  /** The subscriber call back register failed because of some other error */
  ONS_REGISTER_CB_FAILED_GENERAL
} ons_cb_register_status;

/**
 * Registers a call back function for handling notifications for a
 * common subscriber created when ons_init_wconfig_common_ctx() was used
 * to initialize a common ONS client context.
 *
 * Only one call back may be registered at a time for each subscriber
 * Users should not attempt to call ons_subscriber_receive() for a
 * subscriber with a registered call back.
 *
 * ons_subscriber_registercallback_common() may called multiple times
 * for the same subscriber as long as all of the parameters are the same,
 * and ONS will keep a reference count for the registered cb. The subscriber
 * will be unregistered with the matching call to
 * ons_subscriber_cancelcallback() or the final call to ons_subscriber_close().
 *
 * @param[in] s A valid subscriber. If the subscriber was created via
 *              ons_subscriber_create_async(), then its status must be
 *              ONS_SUBSCRIBER_CONNECTED. The subscriber must be for an
 *              ONS client context created via ons_init_wconfig_common_ctx(),
 *              otherwise the call will fail
 * @param[in] cb The call back function that will be called for each
 *               notification received. The call back function <b>must</b>
 *               relinquish each notification via ons_subscriber_relinquish()
 *               before returning. The arguments are:
 * <ul>
 * <li>n The notification for cb to service, which must be relinquished
 *       before cb returns via a call to ons_subscriber_relinquish()</li>
 * <li>ctx The context returned by ctxInit()</li>
 * </ul>
 * @param[in] reference A reference pointer that will be passed to ctxInit()
 *                      and ctxClean()
 * @param[in] ctxInit The function called to allocate and initialize the
 *                    context (ctx) passed to cb. It must not be NULL.
 *                    ctxInit() will only be called on the first call
 *                    to register s, and it must return a context pointer
 *                    (ctx) that will be passed to the call back function cb.
 *                    If ctxInit() returns NULL the registration of cb will
 *                    be aborted. The arguments for ctxInit() are:
 * <ul>
 * <li>reference The reference pointer passed to
 *               ons_subscriber_registercallback_common()</li>
 * <li>s The subscriber passed to ons_subscriber_registercallback_common() --
 *       only ons_subscriber_id() may be used on s from ctxInit()</li>
 * </ul>
 * @param[in] ctxClean The function called to clean ctx. May be NULL. If not
 *                     NULL ctxClean will only be called on the final call to
 *                     ons_subscriber_cancelcallback() or the final call to
 *                     ons_subscriber_close(). If ctxClean() returns 0 then
 *                     no additional action is taken; otherwise
 *                     ons_subscriber_close() is called for the associated
 *                     subscriber if the cancel was not triggered via call
 *                     to close the subscriber, and ons_shutdown_wtimeout_ctx()
 *                     is called for the associated client context.
 *                     The arguments are:
 * <ul>
 * <li>ctx The value returned from ctxInit()</li>
 * <li>reference The reference pointer passed to
 *               ons_subscriber_registercallback_common()</li>
 * <li>s The subscriber passed to ons_subscriber_registercallback_common() --
 *       only ons_subscriber_id() may be used on s from ctxClean()</li>
 * </ul>
 * @param[in] mode The call back mode, which must be either
 * <ul>
 * <li>ONS_ONECBTHREAD which uses the same thread to make all call back
 *                     function calls (the calls will be serialized),</li>
 * <li>ONS_THREADPERCB which creates a new thread as needed for the
 *                     callbacks.</li>
 * </ul>
 *
 * @return One of the following codes:
 *   -  ONS_REGISTER_CB_FIRST - Successful first register cb for s
 *   -  ONS_REGISTER_CB_AGAIN - Successful subsequent register cb for s
 *   -  ONS_REGISTER_CB_FAILED_INIT - Failed to register cb because
 *                                    ctxInit() returned NULL
 *   -  ONS_REGISTER_CB_FAILED_BUSY - Failed to register cb with differing
 *                                    parameters: the subscriber was
 *                                    already registered but with different
 *                                    values for cb, ctxInit or ctxClean
 *   -  ONS_REGISTER_CB_FAILED_ARGS - Failed to register cb because of
 *                                    invalid arguments
 *   -  ONS_REGISTER_CB_FAILED_GENERAL - Failed register cb for another reason
 *
 * @see ons_subscriber_relinquish()
 * @see ons_subscriber_cancelcallback()
 * @see ons_init_wconfig_common_ctx()
 */
ons_cb_register_status
ons_subscriber_registercallback_common(ons_subscriber *s,
  void (*cb)(ons_notification *n, void *ctx),
  void *reference,
  void *(*ctxInit)(void *reference, ons_subscriber *s),
  int (*ctxClean)(void* ctx, void *reference, ons_subscriber *s),
  ons_cb_type mode);

/**
 * Cancels the current registered call back for the given subscriber.
 *
 * ons_subscriber_close() automatically cancels the registered call back.
 *
 * @param[in] s A valid subscriber for which ons_subscriber_registercallback()
 *              has been successfully called.
 *
 * @return 0 if the call back was canceled or -1 if none was registered.
 *
 * @see ons_subscriber_registercallback()
 * @see ons_subscriber_registercallback_common()
 * @see ons_subscriber_close()
 */
int ons_subscriber_cancelcallback(ons_subscriber *s);

/* @} */

/**
 * @defgroup Publisher Publisher creation and operation
 * @ingroup ONS
 *
 * @{
 */

/**
 * Creates a publisher with the specified component id.
 *
 * A publisher is required to publish ONS notifications, and can be created
 * directly via ons_publisher_create() or indirectly by creating a subscriber
 * and getting its publisher via ons_subscriber_getPublisher().
 *
 * @param[in] component A non-NULL component id string.
 *
 * @return The publisher. The publisher resources should be released via
 *         ons_publisher_close().
 *
 * @see ons_publisher_close()
 * @see ons_subscriber_getPublisher()
 */
ons_publisher *ons_publisher_create(const char *component);

/**
 * Creates a publisher with the specified component id for the given
 * ONS context.
 *
 * A publisher is required to publish ONS notifications, and can be created
 * directly via ons_publisher_create_ctx() or indirectly by creating a
 * subscriber and getting its publisher via ons_subscriber_getPublisher().
 *
 * @param[in] context The ONS context returned by ons_init_wconfig_ctx() or
 *                    ons_init_wconfig_common_ctx().
 * @param[in] component A non-NULL component id string.
 *
 * @return The publisher. The publisher resources should be released via
 *         ons_publisher_close().
 *
 * @see ons_init_wconfig_ctx()
 * @see ons_init_wconfig_common_ctx()
 * @see ons_publisher_close()
 * @see ons_subscriber_getPublisher()
 */
ons_publisher* ons_publisher_create_ctx(const void* context,
                 const char* component);

/**
 * Close a publisher obtained via ons_publisher_create() or
 * ons_publisher_create_ctx() and release its resources.
 *
 * @param[in] p A valid publisher returned from ons_publisher_create() or
 *              ons_publisher_create_ctx().
 *
 * @see ons_publisher_create()
 * @see ons_publisher_create_ctx()
 */
void ons_publisher_close(ons_publisher *p);

/**
 * Publish a notification created by ons_notification_create() using
 * a publisher.
 *
 * This function returns when the notification is placed at the tail of
 * the client's send queue.
 *
 * The notification will be sent to the ONS server when it is at the head
 * of the send queue and a connection is established with the ONS server.
 *
 * The notification will have a generatingComponent header with as
 * the publisher's component string.
 *
 * @param[in] p A valid publisher.
 * @param[in] n A valid notification returned by ons_notification_create().
 *
 * @see ons_notification_create()
 */
void ons_publisher_publish(ons_publisher *p, ons_notification *n);

/**
 * Publish a notification created by ons_notification_create() without
 * using a publisher or any other ONS client infrastructure.
 *
 * The information for establishing the connection to the ONS server is
 * obtained using the value of the ORACLE_CONFIG_HOME or ORACLE_HOME
 * environment variable.
 *
 * This API should not be used if the ONS API has been initialized via
 * ons_init() or any of its variations.
 *
 * The function returns when the notification has been sent to the ONS
 * server.
 *
 * @param[in] component A non-NULL component id string used for the value
 *                      of the notification's a generatingComponent header.
 * @param[in] n A valid notification returned by ons_notification_create().
 *
 * @return  One of the following values:
 * - -2 - ORACLE_CONFIG_HOME and ORACLE_HOME are not set or valid or
 *        the client does not have permission to access the path,
 * - -1 - The ONS host and port information could not be parsed, or
 *        a connection to the ONS server could not be established,
 *        or the connection was broken before the notification could
 *        be sent to the server,
 * - 0 -  The notification was sent to the ONS server.
 *
 * @see ons_notification_create()
 */
int ons_one_time_publish(char *component, ons_notification *n);

/**
 * Publish a notification created by ons_notification_create() without
 * using a publisher or any other ONS client infrastructure.
 *
 * The information for establishing the connection to the ONS server is
 * passed in as the host and port arguments. Although the user can specify
 * the ONS server remote host and port values, this API does not support
 * SSL, which is required by default to connect to ONS's remote port.
 * Therefore this API is typically used to connect to the ONS server's
 * local port, for which the host name must be "localhost".
 *
 * This API should not be used if the ONS API has been initialized via
 * ons_init() or any of its variations.
 *
 * The function returns when the notification has been sent to the ONS
 * server.
 *
 * @param[in] component A non-NULL component id string used for the value
 *                      of the notification's a generatingComponent header.
 * @param[in] n A valid notification returned by ons_notification_create().
 * @param[in] host A string specifying the host name for the ONS server.
 * @param[in] port A short integer specifying the port for the ONS server.
 *
 * @return  One of the following values:
 * - -1 - A connection to the ONS server could not be established,
 *        or the connection was broken before the notification could
 *        be sent to the server,
 * - 0 -  The notification was sent to the ONS server.
 *
 * @see ons_notification_create()
 */
int ons_one_time_remote_publish(char *component, ons_notification *n,
      char *host, unsigned short port);

/* @} */

/**
 * @defgroup RPC  Remote Procedure Call
 * @ingroup ONS
 *
 *
 * @{
 *
 * The ONS RPC mechanism is only supported for ONS client configurations with
 * a single server connection; ons_init(), ons_init_woraclehome(), and
 * ons_init_wconfig() only where the configuration string specifies a single
 * server in a single node-list.
 *
 * Any attempt to use the RPC mechanism when the ONS client is initialized with
 * ons_init_wconfig() configured for  multiple server connections is not
 * supported and the results are unpredictable, but may include unreliable
 * request or response handling and thread hangs.
 */

/*
 * A limited set of the eONS RPC functionality is supported by ONS
 */
/**
 * List of registered subscribers.
 */
typedef struct _opmn_list ons_laddrlist;

/**
 * Identifier of a registered subscriber.
 */
typedef struct _ons_logicaladdr ons_logicaladdr;

/**
 * Obtains the route id of the subscriber.
 * The memory for the returned string belongs to the logical
 * address and should not be modified or freed by the caller.
 *
 * @param[in] laddr   a logical address
 *
 * @return the route id
 *
 * This function is unique to ONS.
 */
char* ons_logicaladdr_getRouteId(ons_logicaladdr* laddr);

/**
 * Obtains the name of the component that the subscriber belongs to.
 * Example: "IAS/OHS".  The memory returned belongs to the logical
 * address and should not be freed by the caller.
 *
 * @param[in] laddr   A logical address
 *
 * @return The name of the component
 */
char *ons_logicaladdr_getComponent(ons_logicaladdr *laddr);

/**
 * Obtains the hostname that this subscriber resides on.  NULL.  The memory
 * returned belongs to the logical address and should not be freed by
 * the caller.
 *
 * @param[in] laddr   A logical address
 *
 * @return The hostname
 */
char *ons_logicaladdr_getHostName(ons_logicaladdr *laddr);

/**
 * Obtains the Oracle instance name where the associated subscriber
 * lives.  The memory returned belongs to the logical address and should
 * not be freed by the caller.
 *
 * @param[in] laddr   A logical address
 *
 * @return The Oracle Instance Name
 */
char *ons_logicaladdr_getInstanceName(ons_logicaladdr *laddr);

/**
 * Compares 2 logical addresses.
 *
 * @param[in] laddr1   A logical address
 * @param[in] laddr2   Another logical address
 *
 * @return TRUE if the logical addresses refer to the same subscriber
 */
int ons_logicaladdr_equals(ons_logicaladdr *laddr1, ons_logicaladdr *laddr2);

/**
 * Obtains the the number of properties contained in this logical
 * address.
 *
 * @param[in] laddr   A logical address
 *
 * @return The number of properties in the list
 */
int ons_logicaladdr_getPropertyCount(ons_logicaladdr *laddr);

/**
 * Obtains a property of the given logical address.  If the address
 * is NULL or the index exceeds the value returned from calling
 * ons_logicaladdr_getPropertyCount() then this function will return
 * NULL.  The memory returned belongs to the logical address and should
 * not be freed by the caller.
 *
 * @param[in] laddr     A logical address
 * @param[in] index     The index within the list of properties
 *
 * @return The property or NULL if index >= size of list
 */
ons_namevalue *ons_logicaladdr_getPropertyIndex(ons_logicaladdr *laddr,
  int index);

/**
 * Obtains the value of a component specific property for this
 * logical address.  The memory returned belongs to the logical
 * address and should not be freed by the caller.
 *
 * @param[in] laddr     A logical address
 * @param[in] name      The name of the property to search for
 *
 * @return the value of the named property or NULL if not found
 */
const char *ons_logicaladdr_getProperty(ons_logicaladdr *laddr, char *name);

/**
 * Creates a new Logical Address List.  This call and subsequent additions
 * of logical addresses to the list result in the allocation of memory.
 * When finished with the created list, all associated memory should be
 * freed by making a call to ons_laddrlist_destroy().
 *
 * @return A new logical address list, containing 0 elements
 *
 * @see ons_laddrlist_destroy()
 */
ons_laddrlist *ons_laddrlist_create(void);

/**
 * Frees all memory associated with a Logical Address List.  All logical
 * addresses within the list are freed along with the list itself.
 * This function only needs to be when the logical address list was
 * explicitly created by a call to ons_laddrlist_create().  Logical
 * Address Lists obtained from membership notifications are destroyed
 * when the notification is freed by a call to ons_subscriber_relinquish().
 *
 * @param[in,out] laddrlistp  Pointer to the list to be freed, will be
 *                            set to NULL.
 *
 * @see ons_laddrlist_create()
 */
void ons_laddrlist_destroy(ons_laddrlist **laddrlistp);

/**
 * Obtains the number of Logical Addresses contained in a Logical Address
 * List.  This value can be used to iterate through the logical address
 * list to obtain individual logical addresses using ons_laddrlist_get().
 *
 * @param[in] laddrlist The logical address list
 *
 * @return The number of logical addresses contained in the list or
 *         0 if the list is NULL
 *
 * @see ons_laddrlist_get()
 */
int ons_laddrlist_count(ons_laddrlist *laddrlist);

/**
 * Obtains a Logical Address from a Logical Address List.  The returned
 * logical address remains a part of the logical address list.  The
 * returned logical address should only be referenced as long as the
 * logical address list still exists and has not been destroyed either
 * by implicit destruction as occurs by ons_subscriber_relinquish()
 * or by explicit destruction by ons_laddrlist_destroy().
 *
 * @param[in] laddrlist The list that will receive the logical address
 * @param[in] index     Must be >= 0 and < list size
 *
 * @return The logical address at the specified index or NULL if no
 *         logical address exists at the specified index or if laddrlist
 *         is NULL.
 *
 * @see ons_laddrlist_count()
 */
ons_logicaladdr *ons_laddrlist_get(ons_laddrlist *laddrlist, int index);

/**
 * Adds a Logical Address to a Logical Address List.  The passed in
 * logical address is copied into new memory and added to the list.
 * The original logical address passed in may be freed without harm to
 * the logical address list or the logical addresses contained within.
 * The new copy made of the logical address will be freed when the owning
 * logical address list is freed.  No operation will occur if laddrlist
 * or laddr is NULL.
 *
 * @param[in] laddrlist The list that will receive the logical address
 * @param[in] laddr     A logical address
 *
 * @see ons_laddrlist_destroy()
 */
void ons_laddrlist_add(ons_laddrlist *laddrlist, ons_logicaladdr *laddr);


/**
 * The RPC property list.
 */
typedef struct _opmn_list ons_proplist;

/**
 * Creates a new property list.  This call and subsequent additions
 * of properties to the list result in the allocation of memory.
 * The name/value strings passed in will be copied into new memory.
 * When finished with the created list, all associated memory should be
 * freed by making a call to ons_proplist_destroy().
 *
 * @return a new property list, containing 0 elements
 *
 * @see ons_proplist_destroy()
 * @see ons_rpcserver_setaddrprops()
 */
ons_proplist *ons_proplist_create(void);

/**
 * Frees all memory associated with a property list.  All properties
 * within the list are freed along with the list itself.  This function
 * only needs to be when the property list was explicitly created by a
 * call to ons_proplist_create().
 *
 * @param[in,out] proplistp Pointer to the list to be freed, will be
 *                set to NULL.
 *
 * @see ons_proplist_create()
 */
void ons_proplist_destroy(ons_proplist **proplistp);

/**
 * Adds a property to a property list.  The passed in property attributes
 * are copied into new memory and added to the list.  The new copy made of
 * the property attributes will be freed when the owning property list is
 * freed.  No operation will occur if any of the input arguments are NULL.
 * Duplicate property names are not permitted and newly added properties
 * silently replace existing properties with the same name.
 *
 * The return value of this method can be used to verify that the
 * property was correctly added to the list.
 *
 * @param[in] proplist The list that will receive the property
 * @param[in] name     A property name
 * @param[in] value    A property value
 *
 * Note: the return value type is different than the eONS version.
 *
 * @return 0 if success, -1 if error
 *
 * @see ons_proplist_destroy()
 */
int ons_proplist_put(ons_proplist *proplist, const char *name,
  const char *value);

/**
 * Search by property name for a specific property value.  The memory
 * returned belongs to the property list and should not be freed by
 * the caller.
 *
 * @param[in] proplist  A property list
 * @param[in] name      The name of the property to search for
 *
 * @return The value of the named property.  NULL is returned if the name
 *         is not found, if proplist is NULL, or if name is NULL.
 */
const char *ons_proplist_get(ons_proplist *proplist, const char *name);

/**
 * RPC types
 */
typedef struct _ons_rpcserver     ons_rpcserver;
typedef struct _ons_rpcclient     ons_rpcclient;
typedef struct _ons_rpcinvocation ons_rpcinvocation;
typedef struct _ons_rpcresponse   ons_rpcresponse;
typedef struct _ons_rpcrequest    ons_rpcrequest;

/**
 * Creates an RPC server.  Not yet launched and no RPC function
 * names associated with it.
 *
 * @param[in] servername   The name of this RPC server to be advertised
 *                         for RPC clients to discover.
 * @param[in] component    The component to which this server belongs
 *
 * @return The new RPC server, or NULL on error
 */
ons_rpcserver *ons_rpcserver_create(char *servername, char *component);

/**
 * Registers with an RPC server an RPC function name and an associated
 * callback function which will be invoked as requests arrive.
 *
 * @param[in] rpcserver  The RPC server which will manage the callbacks
 * @param[in] funcname   The name of the RPC to be used by RPC clients
 *                       to reach this particular callback when they find
 *                       the named RPC server.
 * @param[in] callback   The function to call as requests arrive
 *
 * @return 0 if success, -1 if error
 */
typedef void (*ons_rpcserver_cb)(ons_rpcrequest*, char*, int);

int ons_rpcserver_addfunc(ons_rpcserver *rpcserver, char *funcname,
  ons_rpcserver_cb callback);

/**
 * Sets the callback mode to one of ONS_ONECBTHREAD or ONS_THREADPERCB.
 * ONS_ONECBTHREAD uses the same thread to service all callbacks and
 * ONS_THREADPERCB uses a new thread as needed for each callback. The
 * default is ONS_THREADPERCB.
 *
 * @param[in] rpcserver  The RPC server
 * @param[in] cbmode     The name of the RPC to be used by RPC clients
 *                       to reach this particular callback when they find
 *                       the named RPC server.
 *
 * @return 0 if success, -1 if error (RPC server currently launched)
 */
int ons_rpcserver_setcbmode(ons_rpcserver *rpcserver, ons_cb_type cbmode);


/**
 * Called by a user registered callback function: sends the response to
 * the request.
 *
 * @param[in] request      The RPC request for this callback
 * @param[in] response     The response data
 * @param[in] responselen  The length of the response data
 *
 * @return 0 if success, -1 if error
 */
int ons_rpcrequest_sendresult(ons_rpcrequest *request, char *response,
  int responselen);

/**
 * Called by a user registered callback function: sends the partial response to
 * the request.
 *
 * @param[in] request      The RPC request for this callback
 * @param[in] response     The response data
 * @param[in] responselen  The length of the response data
 *
 * @return 0 if success, -1 if error
 */
int ons_rpcrequest_sendprogress(ons_rpcrequest *request, char *response,
int responselen);

/**
 * Called by a user registered callback function: sends an error back to
 * the request.
 *
 * @param[in] request   The RPC request for this callback
 * @param[in] msgcode   The error message code
 * @param[in] msg       The error message
 *
 * @return 0 if success, -1 if error
 */
int ons_rpcrequest_senderror(ons_rpcrequest *request, int msgcode, char *msg);

/**
 * Sets the address properties to be attached to the logical address
 * of the RPC server which gets returned to RPC clients.  This can be
 * used by RPC clients to distinguish between RPC servers according to
 * application specific attributes.  Calling this function is optional.
 * This function must be called prior to calling ons_rpcserver_launch().
 * The passed in ons_proplist is copied and can be modified or freed
 * after returning from this call.
 *
 * @param[in] rpcserver  The RPC server
 * @param[in] addrprops  The user specified additional address
 *                       properties to be added to the RPC server's
 *                       logical address at launch time.
 *
 * @return 0 if success, -1 if error (RPC server currently launched)
 *
 * @see ons_rpcserver_launch()
 * @see ons_proplist_create()
 * @see ons_proplist_put()
 * @see ons_proplist_destroy()
 */
int ons_rpcserver_setaddrprops(ons_rpcserver *rpcserver,
  ons_proplist *addrprops);

/**
 * Launches the RPC Server.  The RPC Server joins the topology and
 * is registered with the system so that RPC clients can discover it.
 * Calls to ons_rpcclient_getservers() will return this server after
 * this call has been made.
 *
 * @param[in] rpcserver  The RPC server
 *
 * @return 0 if success, -1 if error
 *
 * @see ons_rpcserver_shutdown()
 */
int ons_rpcserver_launch(ons_rpcserver *rpcserver);

/**
 * Determines the running status of the RPC server.  The RPC server is
 * running after ons_rpcserver_launch() has been called.
 *
 * @param[in] rpcserver  The RPC server
 *
 * @return 1 if running, 0 if not
 *
 * @see ons_rpcserver_launch()
 * @see ons_rpcserver_shutdown()
 */
int ons_rpcserver_isrunning(ons_rpcserver *rpcserver);

/**
 * Shuts down a running RPC server.  This call has no effect if
 * the server is not running.  This is more gentle than
 * calling ons_rpcserver_destroy() because the RPC server can be
 * launched again without having to recreate the RPC server.
 * Calling this routine or ons_rpcserver_destroy() is recommended
 * prior to shutting down the process as this is how the RPC
 * server is smoothly deregistered from the topology.
 *
 * @param[in] rpcserver  The RPC server
 *
 * @return 0 if success, -1 if error
 *
 * @see ons_rpcserver_launch()
 */
int ons_rpcserver_shutdown(ons_rpcserver *rpcserver);

/**
 * Frees all memory associated with an RPC server.  This call
 * should be made when permanently done with the RPC server.
 * If ons_rpcserver_shutdown has not yet been called, then
 * it will be called prior to freeing all resources.
 *
 * @param[in,out] rpcserverp The RPC server, set to NULL upon return
 *
 * @see ons_rpcserver_create()
 */
void ons_rpcserver_destroy(ons_rpcserver **rpcserverp);

/**
 * Creates a client for the set of servers
 *
 * @param[in] rpcservernames      The RPC servers
 * @param[in] rpcservernamecount  The number of RPC servers
 * @param[in] component           The client component id
 *
 * @return The new RPC client, or NULL on error
 */
ons_rpcclient *ons_rpcclient_create(char **rpcservernames,
  int rpcservernamecount, char *component);

/**
 * Sets the RPC client to operate only with the local OPMN/ONS server. This
 * should be called immediately after ons_rpcclient_create().
 *
 * @param[in] rpcclient      The RPC Client
 */
void ons_rpcclient_setlocal(ons_rpcclient *rpcclient);

/**
 * Sets the RPC client timeouts (when getting logical addresses for servers)
 *
 * @param[in] rpcclient      The RPC Client
 * @param[in] timeout        The timeout (in seconds): minimum value is 2
 *                           and maximum is 120, and the default is 20
 */
void ons_rpcclient_settimeout(ons_rpcclient* rpcclient, int timeout);

/**
 * Obtains the list of currently available RPC Servers matching the
 * specified RPC Server name.  This method will block to get the
 * server list.
 *
 * The returned list needs to be freed by a call to ons_laddrlist_destroy()
 * when finished using it.
 *
 * @param[in] rpcclient      The RPC Client
 * @param[in] rpcServerName  The name of the desired RPC Server(s)
 *
 * @return A list of currently available RPC Servers or NULL if no
 *         RPC Server is available.
 *
 * @see ons_laddrlist_destroy()
 */
ons_laddrlist *ons_rpcclient_getservers(ons_rpcclient *rpcclient,
  char *rpcServerName);

/**
 * Destroys a client
 *
 * @param[in,out] rpcclientp The RPC client, set to NULL upon return
 *
 * @see ons_rpcclient_create()
 */
void ons_rpcclient_destroy(ons_rpcclient **rpcclientp);

/**
 * Invoke an RPC name to the the given list of RPC Servers.
 *
 * The returned invocation needs to be freed by a call to
 * ons_rpcinvocation_destroy() when finished using it.
 *
 * @param[in] rpcclient  The RPC Client
 * @param[in] rpcName    The function name for the listed RPC Server(s)
 * @param[in] laddrlist  The list of RPC Servers
 * @param[in] arg        The argument for the function
 * @param[in] arglen     The length of the argument
 *
 * @return The invocation for this request, or NULL on error.
 *
 * @see ons_rpcclient_getservers()
 * @see ons_rpcinvocation_destroy()
 */
ons_rpcinvocation *ons_rpcclient_invoke(ons_rpcclient *rpcclient,
  char *rpcName, ons_laddrlist *laddrlist, char *arg, int arglen);

/**
 * Invoke an RPC name to the the given list of RPC Servers with
 * a callback function.
 *
 * The returned invocation needs to be freed by a call to
 * ons_rpcinvocation_destroy() when finished using it.
 *
 * @param[in] rpcclient  The RPC Client
 * @param[in] rpcName    The function name for the listed RPC Server(s)
 * @param[in] laddrlist  The list of RPC Servers
 * @param[in] arg        The argument for the function
 * @param[in] arglen     The length of the argument
 * @param[in] callback   The callback function
 * @param[in] ctx        The user supplied context for the callback function
 *
 * @return The invocation for this request, or NULL on error.
 *
 * @see ons_rpcclient_getservers()
 * @see ons_rpcinvocation_destroy()
 */
typedef void (*ons_rpcclient_cb)(ons_rpcresponse*, void*);

ons_rpcinvocation *ons_rpcclient_invoke_wcallback(ons_rpcclient *rpcclient,
    char *rpcName, ons_laddrlist *laddrlist, char *arg, int arglen,
      ons_rpcclient_cb callback, void *ctx);

/**
 * Frees all memory associated with the invocation, including all
 * responses not yet retrieved by a call to ons_rpcinvocation_getresponse().
 *
 * @param[in,out] rpcinvocationp The invocation to be freed, set to NULL
 *                               upon return
 */
void ons_rpcinvocation_destroy(ons_rpcinvocation **rpcinvocationp);

/**
 * Get a single response from an RPC invocation.  Multiple responses
 * may be returned from a single invocation according to the number
 * of server to which the response was directed. The response is valid
 * until ons_rpcinvocation_destroy() is called.
 *
 * @param[in] rpcinvocation Returned when placing an RPC call
 * @param[in] blocking   If 0 then the call will be non-blocking and
 *                       the value of the waittime parameter will be
 *                       ignored.
 * @param[in] waittime   If <= 0 then call will block indefinitely,
 *                       otherwise the call will have a maximum
 *                       wait time of waittime milliseconds.
 * @return The first available RPC response, NULL if no response
 *         is available
 */
ons_rpcresponse *ons_rpcinvocation_getresponse(
  ons_rpcinvocation *rpcinvocation, int blocking, int waittime);

/**
 * Obtains the completion status of an invocation.
 *
 * @param[in] rpcinvocation An invocation that may still be expecting
 *                          more responses
 * @return TRUE if all pending responses have been retrieved
 *         by calls to ons_rpcinvocation_getresponse()
 */
int ons_rpcinvocation_isdone(ons_rpcinvocation *rpcinvocation);

/**
 * Gets the logical address of the RPC server that returned (or was going
 * to return) this response.  This function can be called regardless of
 * whether or not the call succeeded, even if the call never reached the
 * RPC server.
 *
 * @return The logical address of the responding RPC server
 */
ons_logicaladdr *ons_rpcresponse_getaddr(ons_rpcresponse *rpcresponse);

/**
 * Get the response data that the RPC server returned.  The returned
 * memory is valid until ons_rpcinvocation_destroy() is called. If
 * an error occurs then a NULL will be returned and datalen will be 0.
 * An error can be returned when the server was unreachable (went down)
 * or the callback reports an error.  When the RPC server sends back an
 * empty response as part of a successful operation, a valid pointer
 * will be returned and the datalen will be 0.
 *
 * @param[in] rpcresponse  The RPC response
 * @param[out] datalen     The size of the response data in number of bytes
 *
 * @return Pointer to the response from the RPC server. This data has
 *         a NULL byte silently appended to the end in case this is a
 *         string but the NULL byte is not included in the datalen.
 *
 * @see ons_rpcinvocation_getresponse
 * @see ons_rpcresponse_geterror
 */
char *ons_rpcresponse_getdata(ons_rpcresponse *rpcresponse, int *datalen);

/**
 * Frees all memory associated with the response, including response
 * data and error strings.
 *
 * @param[in,out] rpcresponsep Response to be freed, will be set to NULL
 *                             upon return.
 */
void ons_rpcresponse_destroy(ons_rpcresponse **rpcresponsep);

/* @} */

# ifndef WIN32
#ifndef ORATYPES
#include <oratypes.h>
#endif
# endif /* WIN32 */

#include <opmnlist.h>

/**
 * @defgroup BodyBlock  Notification Body Block
 * @ingroup ONS
 *
 * @{
 */

/**
 * Body data identifier.
 *
 * Each body block segment or element is identified by this structure.
 */
typedef struct _opmn_body_ident
{
  char*         obiName;    /* The string name */
  unsigned int  obiLength;  /* The length of the name */
  unsigned int  obiKey;     /* The key for the name */
} opmn_body_ident;

/**
 * Body data element.
 */
typedef struct _opmn_body_elt
{
  opmn_body_ident  obeId;    /* Body element identifier */
  char*            obeData;  /* Body element data */
  unsigned int     obeSize;  /* Size of the body element data */
  unsigned int     obeUser;  /* Body element user defined value */
} opmn_body_elt;

/**
 * Body data segment.
 */
typedef struct _opmn_body_seg
{
  opmn_body_ident  obsId;    /* Body segment identifier */
  opmn_list        obsList;  /* Nested list of body segments/elements */
} opmn_body_seg;

/**
 * Nested data types.
 */
typedef enum _opmn_nest_type
{
  OpmnNestUnknown = 0,
  OpmnNestSegment,
  OpmnNestElement
} opmn_nest_type;

/**
 * Nested data.
 */
typedef union _opmn_nest_data
{
  opmn_body_seg  ondSegment;  /* Segment data */
  opmn_body_elt  ondElement;  /* Element data */
} opmn_nest_data;

/**
 * Nested entry.
 */
typedef struct _opmn_nest_entry
{
  opmn_link       oneLink;  /* Nested entry link for list */
  opmn_nest_type  oneType;  /* Nested entry type */
  opmn_nest_data  oneData;  /* Nested entry data */
} opmn_nest_entry;

#define oneSegment  oneData.ondSegment
#define oneElement  oneData.ondElement

/**
 * ONS Notification standard body data storage and retrieval support.
 *
 * The notification body may contain any arbitrary data, and the ONS
 * client provides the body block mechanism for representing data in an
 * heirarchical tree. An element consists a name value pair. A segment
 * is the equivalent of a directory, and may contain elements or other
 * segments.
 *
 * Users can create a body block and store or retrieve data within.
 * ONS can convert any body block into a binary buffer that can be placed
 * in a notification body. That body data can then be used to recreate
 * the body block.
 */
typedef opmn_list opmn_body_block;

/**
 * Create an empty body block. Called by the sender of the notification for
 * body data management.
 * <p>
 * Usage:
 * </p>
 *   <tt>bodyBlock = opmn_create_body_block();</tt>
 *
 * @return A new empty body block, which must be released via
 *         opmn_destroy_body_block().
 *
 * @see opmn_destroy_body_block()
 */
opmn_body_block* opmn_create_body_block(void);

/**
 * Destroy a body block and any associated resources.
 * <p>
 * Usage:
 * </p>
 *   <tt>opmn_destroy_body_block(bodyBlock);</tt>
 *
 * @param[in]  bodyBlock  The body block to destroy.
 *
 * @see opmn_create_body_block()
 */
void opmn_destroy_body_block(opmn_body_block* bodyBlock);

/**
 * opmn_generate_body: Create the character string encapsulating all
 *                     body block data.
 * <p>
 * Usage:
 * </p>
 *   <tt>body = opmn_generate_body(bodyBlock, &bodylen);</tt>
 *
 * @param[in]   bodyBlock The body block.
 * @param[out]  bodyLen   The length of the returned body string, which can
 *                        be used for the <tt>bodyLen</tt> parameter of
 *                        ons_notification_create().
 *
 * @return The character string for the <tt>body</tt> parameter of
 *         ons_notification_create(). The caller must free this
 *         string when done using it via the free() system call.
 *
 * @see ons_notification_create()
 */
const char* opmn_generate_body(opmn_body_block* bodyBlock,
              unsigned int* bodyLen);

/**
 * Get a body block from the body returned by ons_notification_body().
 * <p>
 * Usage:
 * </p>
 *   <tt>bodyBlock = opmn_get_body_block(body, bodylen);</tt>
 *
 * @param[in]  body     The body string from a notification.
 * @param[in]  bodyLen  The length of the given body string.
 *
 * @return The body block created from the body data.
 *
 * @see ons_notification_body()
 */
opmn_body_block* opmn_get_body_block(const char* body, unsigned int bodyLen);

/**
 * Create a segment with the given name on a segment list. If the segment
 * already exists on the list with the specified name, then the existing
 * segment is used.
 * <p>
 * Usage:
 * </p>
 *   <tt>segList = opmn_put_nested_segment(&parentSeg->obsList, segName);</tt>
 *
 * @param[in]  list     The list on which to search for the segment: this
 *                      may be either a segment list (obsList) or the body
 *                      block itself.
 * @param[in]  segName  The segment name.
 *
 * @return The list for the specified segment.
 */
opmn_list* opmn_put_nested_segment(opmn_list* list, const char* segName);

/**
 * Get the list of nested entries for the named segment on a segment list.
 * <p>
 * Usage:
 * </p>
 *   <tt>segList = opmn_get_nested_segment(&parentSeg->obsList, segName);</tt>
 *
 * @param[in]  list     The list on which to search for the segment: this
 *                      may be either a segment list (obsList) or the body
 *                      block itself.
 * @param[in]  segName  The segment name.
 *
 * @return The list for the specified segment or NULL if the segment was not
 *         found.
 */
opmn_list* opmn_get_nested_segment(opmn_list* list, const char* segName);

/**
 * Put a body element name value pair on the given segment list. If an element
 * already exists on the list with the specified name, then the old element is
 * replaced with the new.
 * <p>
 * Usage:
 * </p>
 *   <tt>opmn_put_nested_element(segList, eltName, eltValue);</tt>
 *
 * @param[in]  list      The list on which to search for the segment: this
 *                       must be a segment list (obsList).
 * @param[in]  eltName   The element name.
 * @param[in]  eltValue  The element value -- this must be a null terminated
 *                       string.
 */
void opmn_put_nested_element(opmn_list* list, const char* eltName,
       const char* eltValue);

/**
 * Get the value of the given element name from the given list.
 * <p>
 * Usage:
 * </p>
 *   <tt>eltValue = opmn_get_nested_element(segList, eltName, &valLen);</tt>
 *
 * @param[in]  list      The list on which to search for the segment: this
 *                       must be a segment list (obsList).
 * @param[in]  eltName   The element name.
 * @param[out] eltLen    The element value length.
 *
 * @return The element value or NULL if the element was not found.
 */
const char* opmn_get_nested_element(opmn_list* list, const char* eltName,
              unsigned int* eltLen);

/**
 * Put a body element name value pair on the first level of the given
 * body block for the named body segment.
 * <p>
 * @deprecated
 * This is an old API provided for backward compatibility with older
 * applications and only supports a flat body block structure (all
 * segments on the same level). See the nested versions of the API
 * for more flexible body block management.
 * </p>
 *
 * @param[in]  bodyBlock  The body block.
 * @param[in]  segName    The segment name.
 * @param[in]  eltName    The element name.
 * @param[in]  eltValue   The element value -- like the names, this must be
 *                        a null terminated string.
 *
 * @see opmn_put_nested_segment()
 * @see opmn_put_nested_element()
 */
void opmn_put_body_element(opmn_body_block* bodyBlock, const char* segName,
       const char* eltName, const char* eltValue);

/**
 * Get the value of the given element name for the named segment in the
 * first level of the body block.
 * <p>
 * @deprecated
 * This is an old API provided for backward compatibility with older
 * applications and only supports a flat body block structure (all
 * segments on the same level). See the nested versions of the API
 * for more flexible body block management.
 * </p>
 *
 * @param[in]  bodyBlock  The body block.
 * @param[in]  segName    The segment name.
 * @param[in]  eltName    The element name.
 * @param[out] eltLen     The element value length.
 *
 * @return The element value, or NULL if the element was not found.
 *
 * @see opmn_get_nested_segment()
 * @see opmn_get_nested_element()
 */
const char* opmn_get_body_element(opmn_body_block* bodyBlock,
              const char* segName, const char* eltName, unsigned int* eltLen);

/**
 * Get the list of all entries for the named segment on the body block.
 * <p>
 * @deprecated
 * This is an old API provided for backward compatibility with older
 * applications and only supports a flat body block structure (all
 * segments on the same level). See the nested versions of the API
 * for more flexible body block management.
 * </p>
 *
 * @param[in]  bodyBlock  The body block.
 * @param[in]  segName    The segment name.
 *
 * @return A list of entries for the named segment on the first level of the
 *         bodyBlock, or NULL if the segment was not found. Note that the type
 *         of entry on the list is <tt>opmn_nest_entry</tt>.
 *
 * @see opmn_get_nested_segment()
 * @see opmn_get_nested_element()
 */
opmn_list* opmn_get_body_segment(opmn_body_block* bodyBlock,
             const char* segName);

/**
 * Generate an opmn_body_ident structure from the given name.
 * <p>
 * This function is useful in conjunction with the <tt>OpmnIdentCompare</tt>
 * macro for matching a specific name with the ids of body elements on the
 * list returned from opmn_get_body_segment().
 * </p>
 * <p>
 * Usage:
 * </p>
 *   <tt>opmn_generate_ident(name, &ident);</tt>
 *
 * @param[in]  name   Name for the ident generation.
 * @param[out] ident  The populated ident structure.
 *
 * @see opmn_get_body_segment()
 */
void opmn_generate_ident(const char* name, opmn_body_ident* ident);

/**
 * OpmnIdentCompare: compare two identifiers.
 * <p>
 * Usage:
 * </p>
 * <pre>
 *   opmn_generate_ident("BarElement", &ident);
 *   if (OpmnIdentCompare(ident, segElt->obsId))
 *   {
 *      Idents matched.
 *   }
 * </pre>
 */
#define OpmnIdentCompare(_i1, _i2)                                            \
  (((_i1).obiKey == (_i2).obiKey) &&                                          \
  ((_i1).obiLength == (_i2).obiLength) &&                                     \
  !strcmp((_i1).obiName, (_i2).obiName))

/**
 * Associate an unkeyed 9.0.2 body segment with the body block. This
 * segment will be placed unformatted at the beginning of the generated
 * body string and is intended as support for 9.0.2 style body processing.
 * <p>
 * Only one old body segment is supported per body block.
 * </p>
 * <p>
 * @deprecated
 * </p>
 *
 * @param[in]  bodyBlock  The body block.
 * @param[in]  segData    The old segment data.
 */
void opmn_put_old_segment(opmn_body_block* bodyBlock, const char* segData);

/**
 * Get the unkeyed 9.0.2 body segment from the body block.
 * <p>
 * @deprecated
 * </p>
 *
 * @param[in]  bodyBlock  The body block.
 * @param[out] segLen     The length of the old segment data.
 *
 * @return The old segment data or NULL if one does not exist in the
 *         body block.
 */
const char* opmn_get_old_segment(opmn_body_block* bodyBlock,
              unsigned int* segLen);

/* @} */

/* @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_ORACLE_ONS_H */
