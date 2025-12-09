idf_component_register(
    SRCS 
        "main.c"
        "config.c"
        "core.c"
    INCLUDE_DIRS 
        ""
    PRIV_REQUIRES
        spiffs
        pthread
)

spiffs_create_partition_image(spiffs ../spiffs_data FLASH_IN_PROJECT)