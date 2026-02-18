set(srcs "main.c")

idf_component_register(
    SRCS
        "main.c"
        "LedControl/LedFunctions.c"
        "BleControl/ble_spp_server.c"
        "Helper/system_state.c"
		"BleControl/gatt_svr.c"
		
    INCLUDE_DIRS
        "."
        "BleControl"
        "LedControl"
        "Helper"
)
