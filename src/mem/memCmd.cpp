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

   bool doArray = false;
   int arrayOpt;
   string arrayCmd;
   for (size_t i = 0, n = options.size(); i < n; ++i) {
     if (myStrNCmp("-Array", options[i], 2) == 0) {
        if (doArray) return CmdExec::errorOption(CMD_OPT_EXTRA, options[i]);
        doArray = true;
        arrayOpt = i;
        arrayCmd = options[i];
     }
   }

   if (options.empty())
      return CmdExec::errorOption(CMD_OPT_MISSING, "");

   if (doArray) {
     if (options.size() == 2) {
       if (arrayOpt == 0) {
         return CmdExec::errorOption(CMD_OPT_MISSING, "");
       } else {
         return CmdExec::errorOption(CMD_OPT_MISSING, options[1]);
       }
     } else if (options.size() == 3) {
       int arraySize;
       int numObjects;
       if (arrayOpt == 0) {
         if (!myStr2Int(options[2], numObjects)) {
           return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
         }
         if (!myStr2Int(options[1], arraySize)) {
           return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
         } else if (arraySize <= 0) {
           return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
         } else {
           try {
             mtest.newArrs(numObjects, arraySize);
           } catch (bad_alloc) {
             return CMD_EXEC_ERROR;
           }
         }
       } else if (arrayOpt == 1) {
         if (!myStr2Int(options[0], numObjects)) {
           return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
         }
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
         return CmdExec::errorOption(CMD_OPT_EXTRA, options[1]);
       }
     } else {
       if (arrayOpt > 1) {
         return CmdExec::errorOption(CMD_OPT_EXTRA, options[1]);
       } else {
         return CmdExec::errorOption(CMD_OPT_EXTRA, options[3]);
       }
     }
   } else {
     int numObjects;
     if (!myStr2Int(options[0], numObjects)) {
       return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
     }
     if (options.size() == 1) {
       if (numObjects < 1) {
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
       } else {
         try {
           mtest.newObjs(numObjects);
         } catch (bad_alloc) {
           return CMD_EXEC_ERROR;
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

   if (options.size() > 3)
      return CmdExec::errorOption(CMD_OPT_EXTRA, options[3]);

   bool checkIdx = false;
   bool checkRdm = false;
   bool checkArr = false;

   int idxOpt;
   int rdmOpt;
   int arrayOpt;

   for (size_t i = 0, n = options.size(); i < n; ++i) {
     if (myStrNCmp("-Index", options[i], 2) == 0) {
       if (checkRdm || checkIdx) return CmdExec::errorOption(CMD_OPT_EXTRA, options[i]);
       checkIdx = true;
       idxOpt = i;
     }
     if (myStrNCmp("-Random", options[i], 2) == 0) {
       if (checkRdm || checkIdx) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
       checkRdm = true;
       rdmOpt = i;
     }
     if (myStrNCmp("-Array", options[i], 2) == 0) {
       if (checkArr) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
       checkArr = true;
       arrayOpt = i;
     }
   }

   if (checkRdm || checkIdx) {
     if (checkArr) {
       if (options.size() == 3) {
         if (arrayOpt == 0) {
           if (rdmOpt == 1 || idxOpt == 1) {
             int num;
             if (myStr2Int(options[2], num) && num >= 0) {
               if (checkIdx) {
                 if (num >= mtest.getArrListSize()) {
                   return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
                 } else {
                   if (mtest.getArrListSize() == 0) {
                     cerr << "Size of array list is 0!!" << endl;
                     return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                   } else {
                     mtest.deleteArr(num);
                   }
                 }
               } else {
                 if (num == 0) {
                   return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
                 } else {
                   if (mtest.getArrListSize() == 0) {
                     cerr << "Size of array list is 0!!" << endl;
                     return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                   } else {
                     for (size_t i = 0; i < num; ++i) {
                       mtest.deleteArr(rnGen(mtest.getArrListSize()));
                     }
                   }
                 }
               }
             }
           } else {
             return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
           }
         } else if (arrayOpt == 1) {
           if (rdmOpt == 0 || idxOpt == 0) {
             return CmdExec::errorOption(CMD_OPT_MISSING, options[0]);
           } else {
             return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
           }
         } else {
           if (rdmOpt == 0 || idxOpt == 0) {
             int num;
             if (myStr2Int(options[1], num) && num >= 0) {
               if (checkIdx) {
                 if (num >= mtest.getArrListSize()) {
                   cerr << "Size of array list (" << mtest.getArrListSize() << ") is <= " << num << "!!" << endl;
                   return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                 } else {
                   if (mtest.getArrListSize() == 0) {
                     cerr << "Size of array list is 0!!" << endl;
                     return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                   } else {
                     if (mtest.getArrListSize() == 0) {
                       cerr << "Size of array list is 0!!" << endl;
                       return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
                     } else {
                       mtest.deleteArr(num);
                     }
                   }
                 }
               } else {
                 if (num == 0) {
                   return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                 } else {
                   if (mtest.getArrListSize() == 0) {
                     cerr << "Size of array list is 0!!" << endl;
                     return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                   } else {
                     if (mtest.getArrListSize() == 0) {
                       cerr << "Size of array list is 0!!" << endl;
                       return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
                     } else {
                       for (size_t i = 0; i < num; ++i) {
                         mtest.deleteArr(rnGen(mtest.getArrListSize()));
                       }
                     }
                   }
                 }
               }
             } else {
               return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
             }

           } else {
             return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
           }
         }
       } else {
         if (rdmOpt == 0 || idxOpt == 0) {
           return CmdExec::errorOption(CMD_OPT_MISSING, options[0]);
         } else {
           return CmdExec::errorOption(CMD_OPT_MISSING, options[1]);
         }
       }
     } else {
       if (options.size() == 1) {
         return CmdExec::errorOption(CMD_OPT_MISSING, options[0]);
       } else if (options.size() == 2) {
         if (rdmOpt == 0 || idxOpt == 0) {
           int num;
           if (myStr2Int(options[1], num) && num >= 0) {
             if (checkIdx) {
               if (num >= mtest.getObjListSize()) {
                 return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
               } else {
                 if (mtest.getObjListSize() == 0) {
                   cerr << "Size of array list is 0!!" << endl;
                   return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
                 } else {
                   mtest.deleteObj(num);
                 }
               }
             } else {
               if (num == 0) {
                 return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
               } else {
                 if (mtest.getObjListSize() == 0) {
                   cerr << "Size of object list is 0!!" << endl;
                   return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
                 } else {
                   for (size_t i = 0; i < num; ++i) {
                     mtest.deleteObj(rnGen(mtest.getObjListSize()));
                   }
                 }
               }
             }
           } else {
             return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
           }
         } else {
           return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
         }
       } else {
         if (rdmOpt == 0 || idxOpt == 0) {
           return CmdExec::errorOption(CMD_OPT_EXTRA, options[2]);
         } else {
           return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
         }
       }
     }
   } else {
     return CmdExec::errorOption(CMD_OPT_MISSING, "");
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
