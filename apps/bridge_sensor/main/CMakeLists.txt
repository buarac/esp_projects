set(COMPONENT_SRCS "bridge_task.c" "bridge_event.c" "bridge_mqtt.c" "bridge_espnow.c" "bridge_wifi.c" "bridge.c")

register_component()

# Create a SPIFFS image from the contents of the 'spiffs_image' directory
# that fits the partition named 'storage'. FLASH_IN_PROJECT indicates that
# the generated image should be flashed when the entire project is flashed to
# the target with 'idf.py -p PORT flash'.
