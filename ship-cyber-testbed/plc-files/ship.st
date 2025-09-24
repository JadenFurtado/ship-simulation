PROGRAM shipplc
    VAR
        fuel_level AT %MD7 : REAL;            
        fuel_temp AT %MD8 : REAL;            
        fuel_pressure AT %MD9 : REAL;        
        start_pump AT %QX1.0 : BOOL;         
        stop_pump AT %QX1.1 : BOOL;          

        engine_power AT %MD11 : REAL;         
        engine_rpm AT %MD12 : REAL;          
        vessel_heading AT %MD13 : REAL;      
        vessel_speed AT %MD14 : REAL;       
    END_VAR


    VAR
        pump_state AT %QX0.0 : BOOL := FALSE;        
        high_temp_alarm AT %QX0.1 : BOOL := FALSE;   
        low_fuel_alarm AT %QX0.2 : BOOL := FALSE;    
        high_pressure_alarm AT %QX0.3 : BOOL := FALSE;
        low_power_alarm AT %QX0.4 : BOOL := FALSE;   
        over_rpm_alarm AT %QX0.5 : BOOL := FALSE;    
        speed_warning AT %QX0.6 : BOOL := FALSE;     
    END_VAR

    VAR
        temp_threshold_reached AT %QX0.7 : BOOL := FALSE;
        LOW_FUEL_THRESHOLD AT %MD1 : REAL := 10.0;       
        HIGH_TEMP_THRESHOLD AT %MD2 : REAL := 80.0;      
        HIGH_PRESSURE_THRESHOLD AT %MD3 : REAL := 5.0;   
        LOW_POWER_THRESHOLD AT %MD4 : REAL := 20.0;      
        HIGH_RPM_THRESHOLD AT %MD5 : REAL := 5000.0;     
        MAX_SPEED AT %MD6 : REAL := 30.0;                
    END_VAR

    IF fuel_level < LOW_FUEL_THRESHOLD THEN
        low_fuel_alarm := TRUE;
    ELSE
        low_fuel_alarm := FALSE;
    END_IF;

    IF fuel_temp > HIGH_TEMP_THRESHOLD THEN
        high_temp_alarm := TRUE;
        temp_threshold_reached := TRUE;
    ELSE
        high_temp_alarm := FALSE;
        temp_threshold_reached := FALSE;
    END_IF;

    IF fuel_pressure > HIGH_PRESSURE_THRESHOLD THEN
        high_pressure_alarm := TRUE;
    ELSE
        high_pressure_alarm := FALSE;
    END_IF;

    IF engine_power < LOW_POWER_THRESHOLD THEN
        low_power_alarm := TRUE;
    ELSE
        low_power_alarm := FALSE;
    END_IF;

    IF engine_rpm > HIGH_RPM_THRESHOLD THEN
        over_rpm_alarm := TRUE;
    ELSE
        over_rpm_alarm := FALSE;
    END_IF;

    IF vessel_speed > MAX_SPEED THEN
        speed_warning := TRUE;
    ELSE
        speed_warning := FALSE;
    END_IF;

    IF start_pump AND NOT stop_pump THEN
        pump_state := TRUE;
    ELSIF stop_pump THEN
        pump_state := FALSE;
    END_IF;
END_PROGRAM


CONFIGURATION Config0

  RESOURCE Res0 ON PLC
    TASK task0(INTERVAL := T#20ms,PRIORITY := 0);
    PROGRAM instance0 WITH task0 : shipplc;
  END_RESOURCE
END_CONFIGURATION