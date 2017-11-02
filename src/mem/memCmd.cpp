/****************************************************************************
  FileName     [ memCmd.cpp ]
  PackageName  [ mem ]
  Synopsis     [ Define memory test commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <iomanip>
#include "memCmd.h"
#include "memTest.h"
#include "cmdParser.h"
#include "util.h"

using namespace std;

extern MemTest mtest;  // defined in memTest.cpp

bool
initMemCmd()
{
   if (!(cmdMgr->regCmd("MTReset", 3, new MTResetCmd) &&
         cmdMgr->regCmd("MTNew", 3, new MTNewCmd) &&
         cmdMgr->regCmd("MTDelete", 3, new MTDeleteCmd) &&
         cmdMgr->regCmd("MTPrint", 3, new MTPrintCmd)
      )) {
      cerr << "Registering \"mem\" commands fails... exiting" << endl;
      return false;
   }
   return true;
}


//----------------------------------------------------------------------
//    MTReset [(size_t blockSize)]
//----------------------------------------------------------------------
CmdExecStatus
MTResetCmd::exec(const string& option)
{
   // check option
   string token;
   if (!CmdExec::lexSingleOption(option, token))
      return CMD_EXEC_ERROR;
   if (token.size()) {
      int b;
      if (!myStr2Int(token, b) || b < int(toSizeT(sizeof(MemTestObj)))) {
         cerr << "Illegal block size (" << token << ")!!" << endl;
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);
      }
      #ifdef MEM_MGR_H
      mtest.reset(toSizeT(b));
      #else
      mtest.reset();
      #endif // MEM_MGR_H
   }
   else
      mtest.reset();
   return CMD_EXEC_DONE;
}

void
MTResetCmd::usage(ostream& os) const
{
   os << "Usage: MTReset [(size_t blockSize)]" << endl;
}

void
MTResetCmd::help() const
{
   cout << setw(15) << left << "MTReset: "
        << "(memory test) reset memory manager" << endl;
}


//----------------------------------------------------------------------
//    MTNew <(size_t numObjects)> [-Array (size_t arraySize)]
//----------------------------------------------------------------------
CmdExecStatus
MTNewCmd::exec(const string& option)
{
   // TODO
   vector<string> options;
   if (!CmdExec::lexOptions(option, options))
      return CMD_EXEC_ERROR;

   if (options.empty())
      return CmdExec::errorOption(CMD_OPT_MISSING, "");

   int numObjects;
   if (!myStr2Int(options[0], numObjects)) {
     return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
   }

   if (options.size() == 1) {
     if (numObjects <= 0) {
       return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
     } else {
       try {
         mtest.newObjs(numObjects);
       } catch (bad_alloc) {
         return CMD_EXEC_ERROR;
       }
     }
   } else if (options.size() > 1) {
     bool doArray = false;
     int arrayOpt;
     for (size_t i = 0, n = options.size(); i < n; ++i) {
       if (myStrNCmp("-Array", options[i], 2) == 0) {
          if (doArray) return CmdExec::errorOption(CMD_OPT_EXTRA, options[i]);
          doArray = true;
          arrayOpt = i;
       }
     }

     if (doArray) {
       if (arrayOpt > 1) {
         return CmdExec::errorOption(CMD_OPT_EXTRA, options[1]);
       } else if (arrayOpt == 1) {
         if (options.size() == 2) {
           return CmdExec::errorOption(CMD_OPT_MISSING, options[1]);
         } else if (options.size() == 3) {
           int arraySize;
           if (!myStr2Int(options[2], arraySize)) {
             return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
           } else if (arraySize <= 0) {
             return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
           } else {
             try {
               mtest.newArrs(numObjects, arraySize);
             } catch (bad_alloc) {
               return CMD_EXEC_ERROR;
             }
           }
         } else {
           return CmdExec::errorOption(CMD_OPT_EXTRA, options[3]);
         }
       }
     } else {
       return CmdExec::errorOption(CMD_OPT_EXTRA, options[1]);
     }
   }
   return CMD_EXEC_DONE;
}

void
MTNewCmd::usage(ostream& os) const
{
   os << "Usage: MTNew <(size_t numObjects)> [-Array (size_t arraySize)]\n";
}

void
MTNewCmd::help() const
{
   cout << setw(15) << left << "MTNew: "
        << "(memory test) new objects" << endl;
}


//----------------------------------------------------------------------
//    MTDelete <-Index (size_t objId) | -Random (size_t numRandId)> [-Array]
//----------------------------------------------------------------------
CmdExecStatus
MTDeleteCmd::exec(const string& option)
{
   // TODO
   vector<string> options;
   if (!CmdExec::lexOptions(option, options))
      return CMD_EXEC_ERROR;

   if (options.empty())
      return CmdExec::errorOption(CMD_OPT_MISSING, "");

   bool checkOpt = false;
   for (size_t i = 0, n = options.size(); i < n; ++i) {
     if (myStrNCmp("-Index", options[i], 2) == 0 || myStrNCmp("-Random", options[i], 2) == 0) {
        checkOpt = true;
     }
   }
   if (checkOpt) {
     if (myStrNCmp("-Index", options[0], 2) != 0 && myStrNCmp("-Random", options[0], 2) != 0) {
        return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
     }
   } else {
     return CmdExec::errorOption(CMD_OPT_MISSING, "");
   }

   if (options.size() == 1) {
      return CmdExec::errorOption(CMD_OPT_MISSING, options[0]);
   } else if (options.size() == 2) {
      int num;
      if (myStr2Int(options[1], num) && num >= 0) {
         if (myStrNCmp("-Index", options[0], 2) == 0) {
            if (num >= mtest.getObjListSize()) {
               return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
            } else {
               mtest.deleteObj(num);
            }
         } else if (myStrNCmp("-Random", options[0], 2) == 0) {
            if (num == 0) {
               return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
            } else {
               mtest.deleteObj(rnGen(mtest.getObjListSize()));
            }
         }
      } else {
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
      }
   } else if (options.size() == 3) {
      if (myStrNCmp("-Array", options[2], 2) != 0) {
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
      } else {
         int num;
         if (myStr2Int(options[1], num) && num >= 0) {
            if (myStrNCmp("-Index", options[0], 2) == 0) {
               if (num >= mtest.getArrListSize()) {
                  return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
               } else {
                  mtest.deleteArr(num);
               }
            } else if (myStrNCmp("-Random", options[0], 2) == 0) {
               if (num == 0) {
                  return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
               } else {
                  mtest.deleteArr(rnGen(mtest.getArrListSize()));
               }
            }
         } else {
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
         }
      }
   } else {
      return CmdExec::errorOption(CMD_OPT_EXTRA, options[3]);
   }
   return CMD_EXEC_DONE;
}

void
MTDeleteCmd::usage(ostream& os) const
{
   os << "Usage: MTDelete <-Index (size_t objId) | "
      << "-Random (size_t numRandId)> [-Array]" << endl;
}

void
MTDeleteCmd::help() const
{
   cout << setw(15) << left << "MTDelete: "
        << "(memory test) delete objects" << endl;
}


//----------------------------------------------------------------------
//    MTPrint
//----------------------------------------------------------------------
CmdExecStatus
MTPrintCmd::exec(const string& option)
{
   // check option
   if (option.size())
      return CmdExec::errorOption(CMD_OPT_EXTRA, option);
   mtest.print();

   return CMD_EXEC_DONE;
}

void
MTPrintCmd::usage(ostream& os) const
{
   os << "Usage: MTPrint" << endl;
}

void
MTPrintCmd::help() const
{
   cout << setw(15) << left << "MTPrint: "
        << "(memory test) print memory manager info" << endl;
}
