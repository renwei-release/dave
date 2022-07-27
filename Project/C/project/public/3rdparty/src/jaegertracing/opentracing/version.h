#ifndef OPENTRACING_VERSION_H
#define OPENTRACING_VERSION_H

#define OPENTRACING_VERSION "1.6.0"
#define OPENTRACING_ABI_VERSION "3"

// clang-format off
#define BEGIN_OPENTRACING_ABI_NAMESPACE \
  inline namespace v3 {
#define END_OPENTRACING_ABI_NAMESPACE \
  }  // namespace v3
// clang-format on

#endif // OPENTRACING_VERSION_H
