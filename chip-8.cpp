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

    // Stack
    unsigned short stack[16];

    public:
        Chip8(std::string rom) {
            initialize();
            loadRom(rom);
            emulateCycle();
        }

    public:
        void emulateCycle() {

            // Fetch opcode, opcode length is 2 bytes hence 2 successive memory reads are required
            opcode = memory[pc] << 8 | memory[pc + 1];
            std::cout << "Opcode read: " << std::hex << opcode << std::endl;
            
            switch (opcode & 0xF000)
            {

                case 0x000:
                    switch (opcode & 0x000F)
                    {
                    case 0x0000: // 0x00E0: Clears the display
                        // TODO: Execute opcode
                        pc += 2;
                        break;

                    case 0x000E: // 0x00EE: Returns from subroutine
                        pc = stack[sp];
                        sp -= 1;
                        pc += 2;
                        break;
                    
                    default:
                        std::cout << "Error unknown opcode: " << std::hex << opcode << std::endl;
                        break;
                    }
                
                case 0x1000: // 1NNN: Jump to location NNN
                    pc = opcode & 0x0FFF;
                    break;

                case 0x2000: // 2NNN: Call subroutine at NNN
                    stack[sp] = pc;
                    sp += 1;
                    pc = opcode & 0x0FFF; 
                    break;
                
                case 0xA000: // ANNN - Sets I to the address NNN 
                    I = opcode & 0x0FFF;
                    pc += 2;
                    break;

                case 0x3000: // 3xkk: Skip next instruction is Vx == kk
                    // unsigned short kk = opcode & 0x00FF;
                    // unsigned short x = (opcode & 0x0F00) >> 2;
                    if ((opcode & 0x00FF) == V[((opcode & 0x0F00) >> 8)]) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;
                
                case 0x4000: // 4xkk: Skip next instruction is Vx != kk
                    // unsigned short kk = opcode & 0x00FF;
                    // unsigned short x = (opcode & 0x0F00) >> 2;
                    if ((opcode & 0x00FF) != V[((opcode & 0x0F00) >> 8)]) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;

                case 0x5000: // 5xy0: Skip next instruction is Vx == Vy
                    if (V[((opcode & 0x0F00) >> 8)] == V[((opcode & 0x00F0) >> 4)]) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;
                
                case 0x6000: // 6xkk: Sets the value kk into register Vx
                    V[((opcode & 0x0F00) >> 8)] = (opcode & 0x00FF);
                    pc += 2;
                    printRegisters();
                    break;
                
                case 0x7000: // 7xkk: Adds the value kk to Vx and stores the value in Vx
                    V[((opcode & 0x0F00) >> 8)] = V[((opcode & 0x0F00) >> 8)] + (opcode & 0x00FF);
                    break;
                case 0x8000: 
                    switch (opcode & 0x000F) {
                        
                        case 0x0000: // 8xy0: Stores the value of Vy in Vx
                            V[((opcode & 0x0F00) >> 8)] = V[((opcode & 0x00F0) >> 4)];
                            pc += 2;
                            break;
                        
                        case 0x0001: // 8xy1: Stores the value of Vx | Vy in Vx
                            V[((opcode & 0x0F00) >> 8)] = V[((opcode & 0x0F00) >> 8)] | V[((opcode & 0x00F0) >> 4)];
                            pc += 2;
                            break;
                        
                        case 0x0002: // 8xy2: Stores the value of Vx & Vy in Vx
                            V[((opcode & 0x0F00) >> 8)] = V[((opcode & 0x0F00) >> 8)] & V[((opcode & 0x00F0) >> 4)];
                            pc += 2;
                            break;
                        
                        case 0x0003: // 8xy3: Stores the value of Vx ^ Vy in Vx
                            V[((opcode & 0x0F00) >> 8)] = V[((opcode & 0x0F00) >> 8)] ^ V[((opcode & 0x00F0) >> 4)];
                            pc += 2;
                            break;
                        
                        case 0x0004: // 8xy4: Sets Vx = Vx + Vy and Vf = 1 if sum > 255 else 0
                            if( (V[((opcode & 0x0F00) >> 8)] + V[((opcode & 0x00F0) >> 4)]) > 255 ) V[15] = 1;
                            else V[15] = 0;
                            V[((opcode & 0x0F00) >> 8)] += V[((opcode & 0x00F0) >> 4)];
                            pc += 2;
                            break;
                        case 0x0005: // 8xy5: Sets Vx = Vx - Vy and Vf = 0 is there is a borrow else Vf = 1
                            if(V[((opcode & 0x00F0) >> 4)] > V[((opcode & 0x0F00) >> 8)]) V[15] = 0;
                            else V[15] = 1;
                            V[((opcode & 0x0F00) >> 8)] -= V[((opcode & 0x00F0) >> 4)];
                            pc += 2;
                            break;
                        case 0x0006: // 8xy6: Sets Vf = 1 if LSB of Vx is 1 else Vf = 0 then Vx = Vx / 2 
                            if ((V[((opcode & 0x0F00) >> 8)] & 1) == 1) V[15] = 1;
                            else V[15] = 0;
                            V[((opcode & 0x0F00) >> 8)] = V[((opcode & 0x0F00) >> 8)] >> 1;
                            pc += 2;
                            break;
                        case 0x0007: // 8xy7: Sets Vx = Vy - Vx and Vf = 0 is there is a borrow else Vf = 1
                            if(V[((opcode & 0x00F0) >> 4)] < V[((opcode & 0x0F00) >> 8)]) V[15] = 0;
                            else V[15] = 1;
                            V[((opcode & 0x0F00) >> 8)] = V[((opcode & 0x00F0) >> 4)] - V[((opcode & 0x0F00) >> 8)];
                            pc += 2;
                            break;
                        case 0x000E: // Sets Vf = 1 if MSB of Vx is 1 else Vf = 0 then Vx = 2 * Vx
                            if ((V[((opcode & 0x0F00) >> 8)] & 0x80) == 1) V[15] = 1;
                            else V[15] = 0;
                            V[((opcode & 0x0F00) >> 8)] = V[((opcode & 0x0F00) >> 8)] << 1;
                            pc += 2;
                            break;
                    }
                    break;
                
                case 0x9000: // 9xy0: Skip next instruction if Vx != Vy
                    if ( V[((opcode & 0x0F00) >> 8)] != V[((opcode & 0x00F0) >> 4)])
                        pc += 4;
                    else
                        pc += 2;
                    break;
                
                case 0xB000:
                    pc = V[0] + (opcode & 0x0FFF);
                    break;

                case 0xC000:
                    break;

                default:
                    std::cout << "Error unknown opcode: " << std::hex << opcode << std::endl;
                    break;
            }

        }

        void initialize() {
            pc = 0x200;
            opcode = 0;
            I = 0;
            sp = 0;
            
            // TODO: Clear display
            
            // Clear stack
            for (int i = 0; i < 16; i++) stack[i] = 0;

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

            for(int i = 512; i < MEM_SIZE; i++) {
                memory[i] = buffer[i - 512];
            }
            // printMemory();
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

        void printRegisters () {
            for (int i = 0; i < 16; i++) {
                std::cout << "Register " << i << ": " << (int) V[i] << std::endl;
            }
        }

};

int main() {

    // Chip8 chip8("pong.rom");
    
    unsigned char t = 0x6F - 0xFF;
    std::cout << (32 >> 1) << std::endl;

    // if ( 0x7F - 0xFF > 255) {
    //     std::cout << "Greater";
    // } else {
    //     std::cout << "Smaller";
    // }

    return 0;
}