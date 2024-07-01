#include <fstream>
#include <string>
#include <iostream>

/*
    Sizes of the data types:
    short - 16 bits / 2 bytes
    int - 32 bits / 4 bytes
*/

class Chip8 {

    const static unsigned short MEM_SIZE = 4096;
    
    unsigned char memory[MEM_SIZE];
    unsigned char chip8_fontset[80];


    // Registers
    unsigned short pc;
    unsigned short opcode;
    unsigned short I;
    unsigned short sp;
    unsigned char V[16];

    public:
        void initialize() {
            pc = 0x200;
            opcode = 0;
            I = 0;
            sp = 0;
            
            // TODO: Clear display
            // TODO: Clear stack
            
            // Clear registers V0-VF
            for (int i = 0; i < 16; i++) V[i] = 0;

            // Clear memory
            for (short i = 0; i < MEM_SIZE; i++) memory[i] = 0;
            
            // Load fontset 
            for (int i = 0; i < 80; ++i) memory[i] = chip8_fontset[i];
            

            // TODO: Reset Timers

        }


        void loadRom(std::string name) {
            std::ifstream inputStream;
            inputStream.open(name, std::ios::binary);
            unsigned char buffer[MEM_SIZE];
            
            // Clear the buffer
            for (int i = 0; i < MEM_SIZE; i++) {
                buffer[i] = 0;
            } 
            inputStream.read(reinterpret_cast<char*>(&buffer), sizeof(buffer));
            // Write from buffer to the main memory
            initialize();
            for(int i = 512; i < MEM_SIZE; i++) {
                memory[i] = buffer[i - 512];
            }
            printMemory();
        }


    // Utility functions
    private:
        void printMemory(){
            for (int i = 0; i < MEM_SIZE; i++)
            {
                std::cout << std::hex << (int) memory[i] << " "; 
            }
            std::cout << std::endl;
        }

};

int main() {
    Chip8 chip8;
    chip8.loadRom("pong.rom");
    return 0;
}