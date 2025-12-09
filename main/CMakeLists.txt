idf_component_register(
    SRCS 
        # Main files
        "main.c"
        "config.c"
        "core.c"
        "file_system.c"

        #LittleFS
        "lib/littlefs/lfs.c"
        "lib/littlefs/lfs_util.c"


    INCLUDE_DIRS 
        "lib/littlefs"

    PRIV_REQUIRES
)

