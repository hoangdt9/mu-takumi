// Stub coreclr_delegates.h for Android — .NET runtime not available on Android.
// This header is included by auto-generated Dotnet/ packet headers.
// On Android, the actual .NET calls are never made (Dotnet .cpp files are excluded from build).
#pragma once
typedef int (*component_entry_point_fn)(void*, int);
typedef int (*load_assembly_and_get_function_pointer_fn)(const char*, const char*, const char*, const char*, void*, void**);
typedef int (*get_function_pointer_fn)(const char*, const char*, const char*, void*, void*, void**);
typedef int (*corehost_error_writer_fn)(const char*);
