metadata {
	// Automatically generated. Make future change here.
	definition (name: "Jody Multi", namespace: "jodyalbritton", author: "Jody Albritton") {
    	capability "Refresh"
   		capability "Polling"
        capability "Temperature Measurement"
        capability "Relative Humidity Measurement"
        capability "Sensor"
        capability "Illuminance Measurement"
        capability "Motion Sensor"
        

		fingerprint profileId: "0104", deviceId: "0138", inClusters: "0000"
	}
    
    

	// Simulator metadata
	simulator {
		// status messages
		status "ping": "catchall: 0104 0000 01 01 0040 00 6A67 00 00 0000 0A 00 0A70696E67"
		status "hello": "catchall: 0104 0000 01 01 0040 00 0A21 00 00 0000 0A 00 0A48656c6c6f20576f726c6421"
	}

	// UI tile definitions
	tiles {
		
        standardTile("motion", "device.motion", width: 2, height: 2) {
			state "active", label:'motion', icon:"st.motion.motion.active", backgroundColor:"#53a7c0"
			state "inactive", label:'no motion', icon:"st.motion.motion.inactive", backgroundColor:"#ffffff"
		}
        valueTile("temperature", "device.temperature", width: 1, height: 1, inactiveLabel: false) {
			state("temperature", label: '${currentValue}°F', unit:"F", backgroundColors: [
                    [value: 31, color: "#153591"],
                    [value: 44, color: "#1e9cbb"],
                    [value: 59, color: "#90d2a7"],
                    [value: 74, color: "#44b621"],
                    [value: 84, color: "#f1d801"],
                    [value: 95, color: "#d04e00"],
                    [value: 96, color: "#bc2323"]
                ]
            )
		}
        valueTile("humidity", "device.humidity", width: 1, height: 1, inactiveLabel: false) {
			state "humidity", label:'${currentValue}% humidity', unit:""
		}
        
        
        
        valueTile("illuminance", "device.illuminance", width: 1, height: 1, inactiveLabel: false) {
			 state "illuminance", label: '${currentValue} ${unit}', unit: 'lux'
			
		}
        standardTile("refresh", "device.poll") {
            state "default", label:'', action:"device.poll()", icon:"st.secondary.refresh"
        }


		main (["motion", "temperature", "humidity","illuminance"])
		details(["motion","temperature","humidity","illuminance"])
	}
}

Map parse(String description) {

	def value = zigbee.parse(description)?.text
	
	// Not super interested in ping, can we just move on? 
	if (value == "ping" || value == " ") 
	{
		return
	}
	
	def linkText = getLinkText(device)
	def descriptionText = getDescriptionText(description, linkText, value)
	def handlerName = value
	def isStateChange = value != "ping"
	def displayed = value && isStateChange
 
	def result = [
		value: value,
		handlerName: handlerName,
		linkText: linkText,
		descriptionText: descriptionText,
		isStateChange: isStateChange,
		displayed: displayed
	]

	if (value in ["!on","!off"])
	{
		result.name  = "switch"
		result.value = value[1..-1]
		
	} else if (value && value[0] == "%") {
		result.name = "level"
		result.value = value[1..-1]
	} else if (value && value == "active") {
		result.name = "motion";
		result.value = "active";
		
    
        	
		
	}else if (value && value == "inactive") {
		result.name = "motion";
		result.value = "inactive";
		
    
        	
		
	} else if (value && value[0] == "h") {
		result.name = "humidity";
		result.value = value[1..-1];
		result.unit = "%"
    
        	
		
	} else if (value && value[0] == "l") {
		result.name = "illuminance";
		result.value = value[1..-1];
		result.unit = "lux"
	} else if (value && value[0] == "t") {
		result.name = "temperature";
		result.value = value[1..-1];
		result.unit = "F"
	
	} else {
		result.name = null; 
	}

 
	if ( (value && value[0] == "%") )
	{
		result.unit = "%"
	
	}
	
	createEvent(name: result.name, value: result.value)
   
	
}



def poll() {
	zigbee.smartShield(text: "poll").format()
}
