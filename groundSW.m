%% Create serial port object 
serialPortNames = serialportlist;
comPort = serialportNames(5); % Set to arduino serial port
% Serial port name is spec
baudRate = 9600; % Set to desired baud rate
arduino = serialport(comPort, baudRate); 

%% Set up the Log File 
logFileName = 'serialLog.txt'; 
logFileId = fopen(logFileName,'w'); % W discards existing file contents

%% UART Message Format
% UART Message Protocol is <ABC>
% A is the address for which subsystem is sending/receiving
%   H = TCS
%   A = ADCS
%   M = Mega
%   T = TTC
%   E = EPS
% B is the Message Type
%   C = Command
%   T = Telemetry
delimiter = ','; % Delimiter character for telemetry data
startChar = '<'; % Start character for message
endChar = '>';   % End character for message
%% Command IDs
% TCS
%   A	ENABLE_HEATERS	Inhibit all heaters
%   B	DISABLE_HEATERS	Unhibit all heaters
%   C	NOMIINAL_SETPOINTS	Standard heater setpoints
%   D	SURVIVAL_SETPOINTS	Survival heater setpoints
% ADCS
%   A	ACTIVATE_RXWHEEL	Send current to motor
%   B	SET_SPEED_RXWHEEL	Set the speed of the motor 
%   C	DISABLE_RXWHEEL	Stop sending current to motor
%   D	SET_IMU_HIGH	Set the IMU sensing state to high 
%   E	SET_IMU_MID	Set the IMU sensing state to mid
%   F	SET_IMU_LOW	Set the IMU sensing state to low 
% MEGA
%   A	RESET_TCS	Reset TCS Arduino UNO
%   B	RESET_ADCS	Reset ADCS Arduino UNO
%   C	RESET_TTC	Reset TTC Arduino UNO
%   D	RESET_ALL	Reset all Arduino UNOs
% EPS
%   A	ENABLE_BATTERY	
%   B	DISABLE_BATTERY	
%   C	ENABLE_RAIL	
%   D	DISABLE_RAIL	

%% Pre allocate some arrays

dataTCS = [];
dataTTC = [];
dataADCS = [];
dataMega = [];
dataEPS = [];
receivedCommands = []; 
tempPanel1All = [];
tempPanel2All = [];
tempPanelAll = [];
tempBreadboardAll = [];
hrCAll = [];
hrVAll = [];
hrPAll = [];
pdiodeAAll = [];
pdiodeBAll = [];
pdiodeCAll = [];
pdiodeDAll = [];
imuXAccelAll = [];
imuYAccelAll = [];
imuZAccelAll = [];

tcsCommandLog = [];
adcsCommandLog = [];
epsCommandLog = [];

% Timing stuff for the while loop 
% TODO: Replace later 
t0 = datetime;
duration = 10; % Duration to run the
t1 = datetime + seconds(10);


%% While loop for reading, parsing, storing serial daa
while datetime < t1         % Runs while loop for 10 seconds
dataLine = readline(arduino); % Read each serial line 
fprintf(logFileId,'%s\n',dataLine);

% Parse the data - make sure it has the appropriate message characters
    if startsWith(dataLine,startChar) && endsWith(dataLine,endChar)
        address = dataLine(2); % Address character from message
        type = dataLine(3);    % Type characacter from message
        messageString = dataLine(4:end-1); % Everything else 

    switch address
        case 'H'    % TCS Message Parsing
            if type == 'T'      % Message is Telemetry Type
                tcsTelemetryLine = str2double(strsplit(messageString,',')); % Split the CSV into cells and make everything class double 
                % TCS Telemetry Format - TODO: Move to config file
                %   ID  Mnemonic            Type    Units
                %   A	PANEL_TEMP_1_C	    Float	degC
                %   B	PANEL_TEMP_2_C	    Float	degC
                %   C	TEMP_BOARD_C	    Float	degC
                %   D	HEATER_RAIL_CURRENT	Float	mA
                %   E	HEATER_RAIL_VOLTAGE	Float	V
                %   F	HEATER_RAIL_POWER	Float	mW
                %   G	HEATERS_ENABLED	    Bool	n/a
                %   H	HEATER_1_ON	        Bool    n/a
                %   I	HEATER_2_ON	        Bool    n/a
                 
                 % Temperature of Panel 1 - Float, degC
                 tempPanel1Latest = tcsTelemetryLine(1);
                 tempPanel1All = [tempPanel1All,tempPanel1Latest];

                 % Temperature of Panel 2 - Float, degC
                 tempPanel2Latest = tcsTelemetryLine(2);
                 tempPanel2All = [tempPanel2All,tempPanel2Latest];

                 % Temperature of Breadboard - Float, degC
                 tempBreadboardLatest = tcsTelemetryLine(3);
                 tempBreadboardAll = [tempBreadboardAll,tempBreadboardLatest];

                 % Heater Rail Current - Float, amps
                 hrCLatest = tcsTelemetryLine(4);
                 hrCAll = [hrCAll,hrCLatest];

                 % Heater Rail Voltage - Float, Volts
                 hrVLatest = tcsTelemetryLine(5);
                 hrVAll = [hrVAll,hrVLatest]; 

                 % Heater Rail Power - Float, Watts
                 hrPLatest = tcsTelemetryLine(6);
                 hrPAll = [hrPAll,hrPLatest]; 

                 % Heaters Enabled - Bool
                 heatersEnabled = logical(tcsTelemetryLine(7));

                 % Temperature of Solar Array Panel - Float, degC
                 tempPanelLatest = tcsTelemetryLine(8);
                 tempPanelAll = [tempPanelAll,tempPanelLatest];

                 % Heater 1 on - Bool
                 hc1Status = logical(tcsTelemetryLine(9)); 

                 % Heater 2 on - Bool
                 hc2Status = logical(tcsTelemetryLine(10)); 
 
            elseif type == 'C'  % Message is Command Type
               
            elseif type == 'L'  % Message is Log type

            else
                disp('The message format type is not understood; expected T, C, or L. Read:', type);
            end
        case 'A'    % ADCS Message Parsing
           if type == 'T'      % Message is Telemetry Type
                ADCSTelemetryLine = str2double(strsplit(messageString,',')); % Split the CSV into cells and make everything class double 
                % ADCS Telmetry Format - TODO: move to config file
                % ID    Mnemonic        Type    Units
                % A	    PDIODE_A	    Float   
                % B	    PDIODE_B	    Float
                % C	    PDIODE_C	    Float
                % D	    PDIODE_D	    Float
                % E	    MOTOR_ON	    Bool
                % F	    IMU_ENABLED	    Bool
                % G	    IMU_X_ACCEL	    Float
                % H	    IMU_Y_ACCEL	    Float
                % I	    IMU_Z_ACCEL	    Float

                % Photodiode A for sun sensor - float
                 pdiodeALatest = ADCSTelemetryLine(1);
                 pdiodeAAll = [pdiodeAAll,pdiodeALatest];

                 % Photodiode B for sun sensor - float
                 pdiodeBLatest = ADCSTelemetryLine(2);
                 pdiodeBAll = [pdiodeBAll,pdiodeBLatest];

                 % Photodiode C for sun sensor - float
                 pdiodeCLatest = ADCSTelemetryLine(3);
                 pdiodeCAll = [pdiodeCAll,pdiodeCLatest];

                 % Photodiode D for sun sensor - float
                 pdiodeDLatest = ADCSTelemetryLine(4);
                 pdiodeDAll = [pdiodeDAll,pdiodeDLatest];

                 % Status of RX wheel motor - bool
                 motorStatus = logical(ADCSTelemetryLine(5)); 

                 % Status of IMU - bool
                 imuStatus = logical(ADCSTelemetryLine(6));

                 % IMU Acceleration in X axis - float
                 imuXAccelLatest = ADCSTelemetryLine(4);
                 imuXAccelAll = [imuXAccelAll,imuXAccelLatest];

            elseif type == 'C'  % Message is Command Type
               
            elseif type == 'L'  % Message is Log type

            else
                disp('The message format type is not understood; expected T, C, or L. Read:', type);
            end

        case 'T'    % TTC Message Parsing

        case 'M'    % Mega Message Parsing
        case 'E'    % EPS Message Parsing
            if type == 'T' % Message is Telemetry Type
            epsTelemetryLine = str2double(strsplit(messageString,',')); % Split the CSV into cells and make everything class double 
            % ID    Mnemonic            Type    Units            
            % A	    BUS_VOLTAGE	        Float	V
            % B	    BUS_CURRENT	        Float	mA
            % C	    BUS_POWER	        Float	mW
            % D	    BATT_ENABLED	    Bool	bool
            % E	    RAIL_ENABLED	    Bool	bool
            % F	    BATT_SOC	        Float	percent
            % G	    BATT_VOLTAGE	    Float	V
            % H	    MPPT_OUT_VOLTAGE	Float	V
            % I	    MPPT_OUT_CURRENT	Float	mA
            % J	    MPPT_OUT_POWER	    Float	mW
            % K	    BATT_CHARGING	    Bool	Bool
            
            elseif type == 'C'
            epsCommandRecieved = strsplit(messageString,',');
            epsCommandLog = [epsCommandLog,epsCommandReceived];
            elseif type == 'L'

            else

                 disp('The message format type is not understood; expected T, C, or L. Read:', type);
            end

    end

    else
        disp('The serial port is recieiving data that is not formatted using the UART Message Protocol: ',dataLine)
    end

%% Other TODOs: 
% Make the script read from the cmd tlm database to create a referencable
%   table


end
