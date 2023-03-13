/* CS 3339 - Fall 2017
 * Project #2
 ******************************/

#include "CPU.h"

const string CPU::regNames[] = {"$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
                                "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
                                "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
                                "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"};

CPU::CPU(uint32_t pc, Memory &iMem, Memory &dMem) : pc(pc), iMem(iMem), dMem(dMem)
{
    for(int i = 0; i < NREGS; i++)
    {
        regFile[i] = 0;
    }

    hi = 0;
    lo = 0;
    regFile[28] = 0x10008000; // gp
    regFile[29] = 0x10000000 + dMem.getSize(); // sp

    instructions = 0;
    stop = false;
}

void CPU::run()
{
    while(!stop)
    {
        instructions++;
        
        stats.clock(IF1);
        fetch();
        decode();
        execute();
        mem();
        writeback();

        D(printRegFile());
    }
}

void CPU::fetch() 
{
    instr = iMem.loadWord(pc);
    pc = pc + 4;
}

void CPU::decode()
{
    uint32_t opcode;      // opcode field
    uint32_t rs, rt, rd;  // register specifiers
    uint32_t shamt;       // shift amount (R-type)
    uint32_t funct;       // funct field (R-type)
    uint32_t uimm;        // unsigned version of immediate (I-type)
    int32_t simm;         // signed version of immediate (I-type)
    uint32_t addr;        // jump address offset field (J-type)

    opcode = instr >> 26;
    rs = (instr >> 21) & 0x1f;
    rt = (instr >> 16) & 0x1f;
    rd = (instr >> 11) & 0x1f;
    shamt = (instr >> 6) & 0x1f;
    funct = instr & 0x3f;
    uimm = instr & 0xffff;
    simm = ((signed)uimm << 16) >> 16;
    addr = instr & 0x3ffffff;

   aluSrc1 = regFile[REG_ZERO];
   aluSrc2 = regFile[REG_ZERO];
   destReg = regFile[REG_ZERO];
   writeDest = false;
   opIsLoad = false;
   opIsStore = false;
   opIsMultDiv = false;
   aluOp = ADD;
   storeData = 0;
  
   D(cout << "  " << hex << setw(8) << pc - 4 << ": ");

   switch(opcode) {
          case 0x00:
              switch(funct) {
                    case 0x00: D(cout << "sll " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                               writeDest = true; destReg = rd;
                               aluOp = SHF_L;
                               stats.registerDest (destReg, MEM1);
                               stats.registerSrc  (rs, EXE1);
                               aluSrc1 = regFile[rs];
                               aluSrc2 = shamt;
                               break;
                    case 0x03: D(cout << "sra " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                               writeDest = true; destReg = rd;
                               aluOp = SHF_R;
                               stats.registerDest (destReg, MEM1);
                               stats.registerSrc (rs, EXE1);
                               aluSrc1 = regFile[rs];
                               aluSrc2 = shamt;
                               break;
                    case 0x08: D(cout << "jr " << regNames[rs]);
                               writeDest = false;
                               stats.registerSrc (rs, ID); 
                               pc = regFile[rs];
                               stats.flush(2);
                               break;
                    case 0x10: D(cout << "mfhi " << regNames[rd]);
                               writeDest = true; destReg = rd;
                               aluOp = ADD;
                               stats.registerDest (destReg, MEM1);
                               stats.registerSrc (REG_HILO, EXE1);
                               aluSrc1 = hi;
                               aluSrc2 = regFile[REG_ZERO];
                               break;
                    case 0x12: D(cout << "mflo " << regNames[rd]);
                               writeDest = true; destReg = rd;
                               aluOp = ADD;
                               stats.registerDest (destReg, MEM1);
                               stats.registerSrc (REG_HILO, EXE1);
                               aluSrc1 = lo;
                               aluSrc2 = regFile[REG_ZERO];
                               break;
                    case 0x18: D(cout << "mult " << regNames[rs] << ", " << regNames[rt]);
                               writeDest = false;
                               opIsMultDiv = true;
                               aluOp = MUL;
                               stats.registerDest (REG_HILO, WB);
                               stats.registerSrc (rs, EXE1);
                               stats.registerSrc (rt, EXE1);
                               aluSrc1 = regFile[rs];
                               aluSrc2 = regFile[rt];
                               break;
                    case 0x1a: D(cout << "div " << regNames[rs] << ", " << regNames[rt]);
                               writeDest = false;
                               opIsMultDiv = true;
                               aluOp = DIV;
                               stats.registerDest (REG_HILO, WB);
                               stats.registerSrc (rs, EXE1);
                               stats.registerSrc (rt, EXE1);
                               aluSrc1 = regFile[rs];
                               aluSrc2 = regFile[rt];
                               break;
                    case 0x21: D(cout << "addu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                               writeDest = true; destReg = rd;
                               aluOp = ADD;
                               stats.registerDest (destReg, MEM1);
                               stats.registerSrc (rs, EXE1);
                               stats.registerSrc (rt, EXE1);
                               aluSrc1 = regFile[rs];
                               aluSrc2 = regFile[rt];
                               break;
                    case 0x23: D(cout << "subu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                               writeDest = true; destReg = rd;
                               aluOp = ADD;
                               stats.registerDest (destReg, MEM1);
                               stats.registerSrc (rs, EXE1);
                               stats.registerSrc (rt, EXE1);
                               aluSrc1 = regFile[rs];
                               aluSrc2 = -regFile[rt];
                               break;
                    case 0x2a: D(cout << "slt " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                               writeDest = true; destReg = rd;
                               aluOp = CMP_LT;
                               stats.registerDest (destReg, MEM1);
                               stats.registerSrc (rs, EXE1);
                               stats.registerSrc (rt, EXE1);
                               aluSrc1 = regFile[rd];
                               aluSrc2 = regFile[rt];
                               break;
                    default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
               }        
               break;
         case 0x02: D(cout << "j " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
                    writeDest = false;
                    pc = (pc & 0xf0000000) | addr << 2;
                    stats.flush(2); 
                    break;
         case 0x03: D(cout << "jal " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
                    writeDest = true;  destReg = REG_RA; // writes PC+4 to $ra
                    aluOp = ADD; // ALU should pass pc thru unchanged
                    stats.registerDest (destReg, EXE1);
                    aluSrc1 = pc;
                    aluSrc2 = regFile[REG_ZERO]; // always reads zero
                    pc = (pc & 0xf0000000) | addr << 2;
                    stats.flush(2);
                    break;
         case 0x04: D(cout << "beq " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
                    stats.registerSrc (rs, ID);
                    stats.registerSrc (rt, ID);
                    if (regFile[rs] == regFile[rt])
                    {
                        pc = pc + (simm << 2);
                        stats.flush(2);
                        stats.countTaken();
                    }
                    stats.countBranch();
                    break;
         case 0x05: D(cout << "bne " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
                    stats.registerSrc (rs, ID);
                    stats.registerSrc (rt, ID);
                    if (regFile[rs] != regFile[rt])
                    {    
                         pc = pc + (simm << 2);
                         stats.flush(2);
                         stats.countTaken();
                    }
                    stats.countBranch();
                    break;
         case 0x09: D(cout << "addiu " << regNames[rt] << ", " << regNames[rs] << ", " << dec << simm);
                    writeDest = true; destReg = rt;
                    aluOp = ADD; 
                    stats.registerDest (destReg, MEM1);
                    stats.registerSrc (rs, EXE1); 
                    aluSrc1 = regFile[rs];
                    aluSrc2 = simm; 
                    break;
         case 0x0c: D(cout << "andi " << regNames[rt] << ", " << regNames[rs] << ", " << dec << uimm);
                    writeDest = true; destReg = rt;
                    aluOp = AND;
                    stats.registerDest (destReg, MEM1);
                    stats.registerSrc (rs, EXE1);
                    aluSrc1 = regFile[rs];
                    aluSrc2 = simm;
                    break;
         case 0x0f: D(cout << "lui " << regNames[rt] << ", " << dec << simm);
                    writeDest = true; destReg = rt; 
                    aluOp = SHF_L; 
                    stats.registerDest (destReg, MEM1);
                    aluSrc1 = simm; 
                    aluSrc2 = 16; 
                    break;
         case 0x1a: D(cout << "trap " << hex << addr);
               switch(addr & 0xf) {
                    case 0x0: cout << endl; break;
                    case 0x1: cout << " " << (signed)regFile[rs];
                              stats.registerSrc (rs, EXE1);
                              break;
                    case 0x5: cout << endl << "? "; cin >> regFile[rt];
                              stats.registerDest(rt, MEM1);
                              break;
                    case 0xa: stop = true; break;
                    default: cerr << "unimplemented trap: pc = 0x" << hex << pc - 4 << endl;
                             stop = true;
               }
               break;
         case 0x23: D(cout << "lw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
                    writeDest = true; destReg = rt;
                    opIsLoad = true;  
                    aluOp = ADD;  
                    stats.registerDest (destReg, WB);
                    stats.registerSrc (rs, EXE1);
                    stats.countMemOp();
                    aluSrc1 = regFile[rs];  
                    aluSrc2 = simm;  
                    break;
         case 0x2b: D(cout << "sw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
                    writeDest = false;
                    opIsStore = true;
                    stats.registerSrc (rs, EXE1);
                    stats.registerSrc (rt, MEM1);
                    stats.countMemOp();
                    storeData = regFile[rt];
                    aluOp = ADD;
                    aluSrc1 = regFile[rs];
                    aluSrc2 = simm;
                    break;
         default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
                    destReg = rt;
      }
      D(cout << endl);
}

void CPU::execute() 
{
   aluOut = alu.op(aluOp, aluSrc1, aluSrc2);
}

void CPU::mem() 
{
  if(opIsLoad)
    writeData = dMem.loadWord(aluOut);
  else
    writeData = aluOut;

  if(opIsStore)
    dMem.storeWord(storeData, aluOut);
}

void CPU::writeback() 
{
  if(writeDest && destReg > 0) // skip if write to reg 0
    regFile[destReg] = writeData;
  
  if(opIsMultDiv) 
  {
    hi = alu.getUpper();
    lo = alu.getLower();
  }
}

void CPU::printRegFile() 
{
   cout << hex;

   for(int i = 0; i < NREGS; i++) 
   {
     cout << "    " << regNames[i];
     if(i > 0) cout << "  ";
         cout << ": " << setfill('0') << setw(8) << regFile[i];
     if( i == (NREGS - 1) || (i + 1) % 4 == 0 )
         cout << endl;
   }
   cout << "    hi   : " << setfill('0') << setw(8) << hi;
   cout << "    lo   : " << setfill('0') << setw(8) << lo;
   cout << dec << endl;
}

void CPU::printFinalStats() 
{
   double totalraw = stats.getRAWHazards(EXE1) + stats.getRAWHazards(EXE2) + stats.getRAWHazards(MEM1) + stats.getRAWHazards(MEM2);

   cout << "Program finished at pc = 0x" << hex << pc << "  ("
        << dec << instructions << " instructions executed)" << endl << endl;

   cout << "Cycles: " << stats.getCycles() << endl
        << "CPI: " << fixed << setprecision(2) << 1.0 * (stats.getCycles() / static_cast<double>(instructions)) << endl << endl;

   cout << "Bubbles: " << stats.getBubbles() << endl
        << "Flushes: " << stats.getFlushes() << endl << endl;

   cout << "Mem ops: " << fixed << setprecision(1) << 100.0 * stats.getMemOps() / instructions << "%"
        << " of instructions" << endl 
        << "Branches: " << 100.0 * stats.getBranches() / instructions << "% of instructions" << endl 
        << " %" << " Taken: " << 100.0 * stats.getTaken() / stats.getBranches() << endl << endl;

   cout << "RAW hazards: " << setprecision(0) << totalraw << " (1 per every " << fixed << setprecision(2) 
        << 1.0 * (static_cast<double>(instructions) / totalraw) << " instructions)" <<  endl
        << " On EXE1 op: " << stats.getRAWHazards(EXE1) << fixed << setprecision(0) << " (" 
        << 100.0 * (stats.getRAWHazards(EXE1) / totalraw) << "%)" << endl
        << " On EXE2 op: " << stats.getRAWHazards(EXE2) << " (" << 100.0 * (stats.getRAWHazards(EXE2) / totalraw) << "%)" << endl
        << " On MEM1 op: " << stats.getRAWHazards(MEM1) << " (" << 100.0 * (stats.getRAWHazards(MEM1) / totalraw) << "%)" << endl
        << " On MEM2 op: " << stats.getRAWHazards(MEM2) << " (" << 100.0 * (stats.getRAWHazards(MEM2) / totalraw) << "%)" << endl; 
}
