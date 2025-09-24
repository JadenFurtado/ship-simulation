# from flask import Flask, request, Response, render_template

import yaml
import struct
import json
from flask import Flask, render_template, request
from flask_socketio import SocketIO, emit
from pymodbus.client.sync import ModbusTcpClient
from pymodbus.constants import Endian
from pymodbus.payload import BinaryPayloadDecoder, BinaryPayloadBuilder

class ModbusReader:
    def __init__(self) -> None:
        # Create a Modbus client
        self.client = ModbusTcpClient('localhost')
        # Connect to the client
        self.client.connect()

    def read_from_registers(self,register):
    # Read holding registers starting at address 100, reading 2 registers
        response = self.client.read_holding_registers(register, count=2, unit=1)
        return response.registers
    
    def read_from_coil(self,coil):
        response = self.client.read_coils(coil,1)
        return response.bits[0]
    
    def read_float_from_registers(self,register):
    # Check if the response is valid register 2052
        response = self.client.read_holding_registers(register, count=2, unit=1)
        if response.isError():
            print("Error reading holding registers")
        else:
            # print("Register values:", response.registers)
            decoder = BinaryPayloadDecoder.fromRegisters(response.registers, Endian.Big, wordorder=Endian.Big)
            # print(decoder.decode_32bit_float())
            return decoder.decode_32bit_float()
    
    def write_to_register(self,register,data):
        binary_value = struct.pack('>f', data)  # '>f' means big-endian float
        # Convert 32-bit binary into two 16-bit integers (registers)
        registers =  struct.unpack('>HH', binary_value)
        response = self.client.write_registers(register,registers)
        return response

app = Flask(__name__,static_url_path='', 
            static_folder='static',
            template_folder='templates')
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app)

with open('config.yaml','r') as file:
    app.config['modbus_config'] = yaml.safe_load(file)

modbus_reader = ModbusReader()

# Serve the index page
@app.route('/')
def index():
    return render_template('index.html')


@app.route('/gui',methods=['GET','POST'])
def gui():
    if request.method=='GET':
        return render_template('power-settings.html')
    elif request.method=='POST':
        power_settings = request.form.get('power_settings')
        if power_settings == '':
            power_settings = 0.0
        modbus_reader.write_to_register(2072,float(power_settings))
        return render_template('power-settings.html')

@app.route('/guages',methods=['GET'])
def guages():
    return render_template('guages.html')

@app.route('/lat-long',methods=['GET'])
def lat_long():
    return render_template('lat-long.html')

# Handle message received from the client (frontend)
@socketio.on('message_from_client')
def handle_message(data):
    print(f"Received from frontend: {data}")
    plc_data = {}
    for variable in app.config['modbus_config']['variables']:
        # print(variable)
        if variable['type']=="holding_registers":
            # print(f"{variable['name']}:{modbus_reader.read_float_from_registers(variable['address'])}")
            plc_data[variable['name']] = modbus_reader.read_float_from_registers(variable['address'])
        elif variable['type']=='discrete_output_coils':
            # print(f"{variable['name']}:{modbus_reader.read_from_coil(variable['address'])}")
            plc_data[variable['name']] = modbus_reader.read_from_coil(variable['address'])
    plc_data['power'] = 1
    # Here, you can process the data or send it back to the frontend
    emit('message_from_server', {'response': f"{json.dumps(plc_data)}"})

# Handle message received from the client (frontend)
@socketio.on('control_settings')
def set_power_data(data):
    plc_data = {}
    plc_data['power'] = 2
    # Here, you can process the data or send it back to the fron tend
    emit('controls', {'response': f"{json.dumps(plc_data)}"})



if __name__ == '__main__':
    socketio.run(app, debug=True)

