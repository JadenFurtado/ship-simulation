package com.example.modbus;

import com.ghgande.j2mod.modbus.Modbus;
import com.ghgande.j2mod.modbus.io.ModbusTCPTransaction;
import com.ghgande.j2mod.modbus.msg.ReadMultipleRegistersRequest;
import com.ghgande.j2mod.modbus.msg.ReadMultipleRegistersResponse;
import com.ghgande.j2mod.modbus.msg.WriteSingleRegisterRequest;
import com.ghgande.j2mod.modbus.net.TCPMasterConnection;
import com.ghgande.j2mod.modbus.procimg.SimpleRegister;

import java.net.InetAddress;

class PeopleCounter {
    static {
        System.loadLibrary("peoplecounter");
    }
    // public native int testMethod(String input);
    public native int detectPeople(String imagePath);

    public static void main(String[] args) {
        PeopleCounter p = new PeopleCounter();
        int result = p.detectPeople("test-images/example2.png");
    //     int result = p.testMethod("test");
        System.out.println("Test result: " + result);
    }
}

public class ModbusClientExample {

    public static void main(String[] args) {
        String modbusHost = "172.17.0.2";
        int modbusPort = 5020;

        TCPMasterConnection connection = null;

        try {
            InetAddress address = InetAddress.getByName(modbusHost);
            connection = new TCPMasterConnection(address);
            connection.setPort(modbusPort);
            connection.connect();
            System.out.println("Connected to Modbus server.");

            int startAddress = 0;
            int count = 10;

            // 1️⃣ Read registers
            ReadMultipleRegistersRequest readRequest = new ReadMultipleRegistersRequest(startAddress, count);
            ModbusTCPTransaction readTransaction = new ModbusTCPTransaction(connection);
            readTransaction.setRequest(readRequest);
            readTransaction.execute();

            ReadMultipleRegistersResponse readResponse = (ReadMultipleRegistersResponse) readTransaction.getResponse();

            // 2️⃣ Call PeopleCounter
            PeopleCounter p = new PeopleCounter();
            int peopleDetected = p.detectPeople("test-images/example2.png");

            // 3️⃣ Loop through registers and update if needed
            for (int i = 0; i < readResponse.getWordCount(); i++) {
                int initialValue = readResponse.getRegister(i).getValue();
                int finalValue = initialValue;

                // Update if detectPeople returns more than 1
                if (peopleDetected > 1 & i>3) {
                    finalValue = initialValue+1;

                    // Write back to Modbus
                    WriteSingleRegisterRequest writeRequest =
                            new WriteSingleRegisterRequest(startAddress + i, new SimpleRegister(finalValue));
                    ModbusTCPTransaction writeTransaction = new ModbusTCPTransaction(connection);
                    writeTransaction.setRequest(writeRequest);
                    writeTransaction.execute();
                }

                // Print the 3 things
                System.out.printf("Register %d - Initial: %d, PeopleDetected: %d, Final: %d%n",
                        startAddress + i, initialValue, peopleDetected, finalValue);
            }

        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (connection != null && connection.isConnected()) {
                connection.close();
                System.out.println("Connection closed.");
            }
        }
    }
}
