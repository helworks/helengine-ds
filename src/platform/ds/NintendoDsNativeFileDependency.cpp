#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include "helcpp_config.hpp"

// Native content sources call File::OpenRead even when managed reachability does not retain File.
// Supply the original runtime implementation once, keeping its configured filesystem provider.
#if !defined(HE_CPP_REQ_FILE)
#define HE_CPP_REQ_FILE 1
#include "system/io/file.cpp"
#endif
#endif
