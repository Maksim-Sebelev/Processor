// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <malloc.h>
// #include <sys/stat.h>
// #include <assert.h>
// #include "Compiler.h"
// #include "../Common/GlobalInclude.h"


// struct Label
// {
//     char* Name;
//     int   CodePlace;    
// };


// struct LabelsTable
// {
//     Label*  Labels;
//     size_t  FirstFree;
//     size_t  Capacity;
// };


// struct CmdDataForAsm
// {
//     FILE*         ProgrammFilePtr;
//     const char*   ProgrammFileName;
//     size_t        ProgrammFileLen;

//     FILE*         CodeFilePtr;

//     int           FileCmdQuant;
//     const char*   Cmd;
//     char**        CmdArr;
//     size_t        ProgrammCmdQuant;

//     size_t        Cmd_i;
//     int*          CmdCodeArr;
//     size_t        CmdCodeArr_i;

//     LabelsTable   Labels;
// };


// struct CmdFunc
// {
//     const char*         CmdName;
//     CompilerErrorType (*CmdFunc) (CmdDataForAsm*, CompilerErrorType*);
// };


// static  void               PrintError   (CompilerErrorType* err);
// static  void               PrintPlace   (const char* file, int line, const char* func);
// static  void               ErrPlaceCtor (CompilerErrorType* err, const char* file, int line, const char* func);
// static  CompilerErrorType  Verif        (CompilerErrorType* err, const char* file, int line, const char* func);

// static  CompilerErrorType  ReadCmdFromFile          (CmdDataForAsm* Cmd);
// static  CompilerErrorType  WriteCmdCodeArrInFile    (CmdDataForAsm* Cmd);
// static  char               GetBufferElem            (char* buffer, size_t buffer_i);
// static  char*              GetBufferElemPtr         (char* buffer, size_t buffer_i);
// static  char*              GetNextCmd               (CmdDataForAsm* Cmd);
// static  void               SetCmdArr                (CmdDataForAsm* Cmd, size_t* CmdArr_i, char* SetElem);
// static  void               SetProgrammCmdQuant      (CmdDataForAsm* Cmd, size_t SetElem);
// static  void               SetCmdArrCodeElem        (CmdDataForAsm* Cmd, int SetElem);
// static  size_t             CalcFileLen              (const char* FileName);
// static  void               CloseFiles               (FILE* ProgrammFilePtr, FILE* CodeFilePtr);

// static  void               LabelCtor          (Label* Lab, const char* Name, int CodePlace);
// static  void               LabelDtor          (Label* Lab);
// static  int                IsLabelInLabels    (const LabelsTable* Labels, const char* LabelName);
// static  CompilerErrorType  LabelsTableCtor    (LabelsTable* Labels);
// static  CompilerErrorType  LabelsTableDtor    (LabelsTable* Labels);
// static  CompilerErrorType  PushLabel          (LabelsTable* Labels, const Label* Lab);
// static  int                GetLabelCodePlace  (CmdDataForAsm* CmdInfo, int Labels_i);
// static  CompilerErrorType  FindLabels         (CmdDataForAsm* Cmd);

// static  CompilerErrorType  NullArgCmdPattern  (CmdDataForAsm* CmdInfo, Cmd Cmd);
// static  CompilerErrorType  JmpCmdPattern      (CmdDataForAsm* CmdInfo, Cmd JumpType, CompilerErrorType* err);

// static  int                GetPushArg    (PushType* Push);
// static  int                GetPopArg     (PopType*  Pop );
// static  CompilerErrorType  PushTypeCtor  (PushType* Push, uint8_t Stk, uint8_t Reg, uint8_t Mem);
// static  CompilerErrorType  PopTypeCtor   (PopType*  Pop , uint8_t Reg, uint8_t Mem);

// static  CompilerErrorType  CmdInfoCtor  (CmdDataForAsm* CmdInfo, LabelsTable* Labels, FILE* ProgrammFilePtr, const char* ProgrammFileName, FILE* CodeFilePtr);
// static  CompilerErrorType  CmdInfoDtor  (CmdDataForAsm* CmdInfo);

// static  bool  IsInt       (const char* str, const char* StrEnd, const size_t StrSize);
// static  bool  IsRegister  (const char* str, const size_t StrSize);
// static  bool  IsMemory    (const char* str, const size_t StrSize);
// static  bool  IsLabel     (const char* str);

// static  const char*        GetCmdName              (size_t Cmd_i);
// static  CompilerErrorType  (*GetCmd(size_t Cmd_i)) (CmdDataForAsm* CmdInfo, CompilerErrorType* err);
// static  CompilerErrorType  UpdateBufferForMemory   (char** buffer, size_t* BufferSize);

// static  CompilerErrorType  HandlePush          (CmdDataForAsm* CmdInfo, CompilerErrorType* err);
// static  CompilerErrorType  HandlePop           (CmdDataForAsm* CmdInfo, CompilerErrorType* err);
// static  CompilerErrorType  HandleJmp           (CmdDataForAsm* CmdInfo, CompilerErrorType* err);
// static  CompilerErrorType  HandleJa            (CmdDataForAsm* CmdInfo, CompilerErrorType* err);
// static  CompilerErrorType  HandleJae           (CmdDataForAsm* CmdInfo, CompilerErrorType* err);
// static  CompilerErrorType  HandleJb            (CmdDataForAsm* CmdInfo, CompilerErrorType* err);
// static  CompilerErrorType  HandleJbe           (CmdDataForAsm* CmdInfo, CompilerErrorType* err);
// static  CompilerErrorType  HandleJe            (CmdDataForAsm* CmdInfo, CompilerErrorType* err);
// static  CompilerErrorType  HandleJne           (CmdDataForAsm* CmdInfo, CompilerErrorType* err);
// static  CompilerErrorType  HandleAdd           (CmdDataForAsm* CmdInfo, CompilerErrorType* err);
// static  CompilerErrorType  HandleSub           (CmdDataForAsm* CmdInfo, CompilerErrorType* err);
// static  CompilerErrorType  HandleMul           (CmdDataForAsm* CmdInfo, CompilerErrorType* err);
// static  CompilerErrorType  HandleDiv           (CmdDataForAsm* CmdInfo, CompilerErrorType* err);
// static  CompilerErrorType  HandleOut           (CmdDataForAsm* CmdInfo, CompilerErrorType* err);
// static  CompilerErrorType  HandleHlt           (CmdDataForAsm* CmdInfo, CompilerErrorType* err);
// static  CompilerErrorType  HandleLabel         (CmdDataForAsm* CmdInfo, CompilerErrorType* err);


// static  CompilerErrorType  RunAssembler  (CmdDataForAsm* CmdInfo);


// static const CmdFunc CmdFuncArr[] =
// {
//     {"push",  HandlePush},
//     {"pop" ,  HandlePop },
//     {"jmp" ,  HandleJmp },
//     {"ja"  ,  HandleJa  },
//     {"jae" ,  HandleJae },
//     {"jb"  ,  HandleJb  },
//     {"jbe" ,  HandleJbe },
//     {"je"  ,  HandleJe  },
//     {"jne" ,  HandleJne },
//     {"add" ,  HandleAdd },
//     {"sub" ,  HandleSub },
//     {"mul" ,  HandleMul },
//     {"div" ,  HandleDiv },
//     {"out" ,  HandleOut },
//     {"hlt" ,  HandleHlt },
// };

// static const size_t CmdFuncQuant = sizeof(CmdFuncArr) / sizeof(CmdFuncArr[0]);

// //--------------------------------------------------------------------------------------------------------------------------------------------------

// CompilerErrorType RunCompiler(const IOfile* file)
// {
//     assert(file);

//     CompilerErrorType err = {};
//     COMPILER_RETURN_IF_ERR(err);

//     FILE* ProgrammFilePtr = fopen(file->ProgrammFile, "rb");
//     FILE* CodeFilePtr     = fopen(file->CodeFile, "wb");

//     if (!ProgrammFilePtr)
//     {
//         err.FailedOpenProgrammFile = 1;
//         err.IsFatalError = 1;
//         return COMPILER_VERIF(err);
//     }

//     if (!CodeFilePtr)
//     {
//         err.FailedOpenCodeFile = 1;
//         err.IsFatalError = 1;
//         return COMPILER_VERIF(err);
//     }


//     LabelsTable Labels = {};
//     // LabelsTableCtor(&Labels);

//     CmdDataForAsm CmdInfo = {};

//     COMPILER_RETURN_IF_ERR(CmdInfoCtor(&CmdInfo, &Labels, ProgrammFilePtr, file->ProgrammFile, CodeFilePtr));

//     COMPILER_RETURN_IF_ERR(ReadCmdFromFile(&CmdInfo));

//     // COMPILER_RETURN_IF_ERR(FindLabels(&CmdInfo));

//     COMPILER_RETURN_IF_ERR(RunAssembler(&CmdInfo));

//     COMPILER_RETURN_IF_ERR(WriteCmdCodeArrInFile(&CmdInfo));

//     COMPILER_RETURN_IF_ERR(CmdInfoDtor(&CmdInfo));
//     // LabelsTableDtor(&Labels); 
//     CloseFiles(ProgrammFilePtr, CodeFilePtr);

//     return COMPILER_VERIF(err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType RunAssembler(CmdDataForAsm* CmdInfo)
// {
//     assert(CmdInfo);

//     CompilerErrorType err = {};

//     CmdInfo->CmdCodeArr = (int*) calloc(100, sizeof(int));

//     while (CmdInfo->Cmd_i < CmdInfo->ProgrammCmdQuant)
//     {
//         CmdInfo->Cmd = GetNextCmd(CmdInfo);
//         bool WasCorrectCmd = false;

//         for (size_t CmdFuncArr_i = 0; CmdFuncArr_i < CmdFuncQuant; CmdFuncArr_i++)
//         {
//             const char* CmdName = GetCmdName(CmdFuncArr_i);

//             if (strcmp(CmdName, CmdInfo->Cmd) == 0)
//             {
//                 WasCorrectCmd = true;
//                 COMPILER_RETURN_IF_ERR(GetCmd(CmdFuncArr_i)(CmdInfo, &err));
//                 break;
//             }

//             if (IsLabel(CmdInfo->Cmd))
//             {
//                 WasCorrectCmd = true;
//                 break;
//             }
//         }

//         if (!WasCorrectCmd)
//         {
//             err.InvalidCmd = 1;
//             err.IsFatalError = 1;
//             return COMPILER_VERIF(err);
//         }
//     }

//     for (int i = 0; i < CmdInfo->FileCmdQuant; i++)
//     {
//         printf("%d ", CmdInfo->CmdCodeArr[i]);
//     }
//     return COMPILER_VERIF(err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType HandleLabel(CmdDataForAsm* CmdInfo, CompilerErrorType* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     size_t LabelSize = strlen(CmdInfo->Cmd);

//     if (CmdInfo->Cmd[LabelSize - 1] != ':')
//     {
//         err->InvalidCmd = 1;
//         err->IsFatalError = 1;
//         return COMPILER_VERIF(*err);
//     }

//     Label Temp = {};
//     LabelCtor(&Temp, CmdInfo->Cmd, CmdInfo->FileCmdQuant); 

//     int LabelIndex = IsLabelInLabels(&CmdInfo->Labels, CmdInfo->Cmd);
    
//     if (LabelIndex == -1)
//     {
//         COMPILER_RETURN_IF_ERR(PushLabel(&CmdInfo->Labels, &Temp));
//         LabelDtor(&Temp);
//         return COMPILER_VERIF(*err);
//     }
    
//     LabelDtor(&Temp);

//     err->MoreOneEqualLables = 1;
//     err->IsFatalError = 1;


//     return COMPILER_VERIF(*err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType HandlePush(CmdDataForAsm* CmdInfo, CompilerErrorType* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     char* buffer = GetNextCmd(CmdInfo);
//     size_t BufferLen = strlen(buffer);
//     char* EndBuffer = nullptr;

//     StackElem_t PushElem = 0;
//     PushElem = (StackElem_t) strtol(buffer, &EndBuffer, 10);

//     PushType Push = {};

//     if (IsInt(buffer, EndBuffer, BufferLen))
//     {
//         COMPILER_ASSERT(PushTypeCtor(&Push, 1, 0, 0));
//         SetCmdArrCodeElem(CmdInfo, push);
//         SetCmdArrCodeElem(CmdInfo, GetPushArg(&Push));
//         SetCmdArrCodeElem(CmdInfo, PushElem);
//     }

//     else if (IsRegister(buffer, BufferLen))
//     {
//         COMPILER_ASSERT(PushTypeCtor(&Push, 0, 1, 0));
//         SetCmdArrCodeElem(CmdInfo, push);
//         SetCmdArrCodeElem(CmdInfo, GetPushArg(&Push));
//         SetCmdArrCodeElem(CmdInfo, buffer[0] - 'a');
//     }

//     else if (IsMemory(buffer, BufferLen))
//     {
//         COMPILER_RETURN_IF_ERR(UpdateBufferForMemory(&buffer, &BufferLen);)

//         StackElem_t PushElemMemIndex = 0;
//         PushElemMemIndex = (StackElem_t) strtol(buffer, &EndBuffer, 10);

//         if (IsInt(buffer, EndBuffer, BufferLen))
//         {
//             COMPILER_ASSERT(PushTypeCtor(&Push, 0, 0, 1));
//             SetCmdArrCodeElem(CmdInfo, push);
//             SetCmdArrCodeElem(CmdInfo, GetPushArg(&Push));
//             SetCmdArrCodeElem(CmdInfo, PushElemMemIndex);
//         }

//         else if (IsRegister(buffer, BufferLen))
//         {
//             COMPILER_ASSERT(PushTypeCtor(&Push, 0, 1, 1));
//             SetCmdArrCodeElem(CmdInfo, push);
//             SetCmdArrCodeElem(CmdInfo, GetPushArg(&Push));
//             SetCmdArrCodeElem(CmdInfo, buffer[0] - 'a');
//         }
//     }

//     else
//     {
//         err->InvalidInputAfterPush = 1;
//         err->IsFatalError = 1;
//         FREE(buffer);
//         return COMPILER_VERIF(*err);
//     }

//     CmdInfo->FileCmdQuant += 3;

//     return COMPILER_VERIF(*err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static bool IsInt(const char* str, const char* StrEnd, const size_t StrSize)
// {
//     assert(str);
//     assert(StrEnd);
//     return StrEnd - str == (int) (StrSize * sizeof(str[0]));
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static bool IsRegister(const char* str, const size_t StrSize)
// {
//     assert(str);
//     return StrSize == 2 && 'a' <= str[0] && str[0] <= 'd' &&  str[1] == 'x';
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static bool IsMemory(const char* str, const size_t StrSize)
// {
//     assert(str);
//     return str[0] == '[' && str[StrSize - 1] == ']';
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static bool IsLabel(const char* str)
// {
//     assert(str);
//     return str[strlen(str) - 1] == ':';
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static const char* GetCmdName(size_t Cmd_i)
// {
//     return CmdFuncArr[Cmd_i].CmdName;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType (*GetCmd(size_t Cmd_i)) (CmdDataForAsm* CmdInfo, CompilerErrorType* err)
// {
//     return CmdFuncArr[Cmd_i].CmdFunc;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType UpdateBufferForMemory(char** buffer, size_t* BufferSize)
// {
//     assert(buffer);
//     assert(BufferSize);

//     CompilerErrorType err = {};
//     (*buffer)[*BufferSize - 1] = '\0';
//     *BufferSize -= 2;
//     (*buffer)++;

//     return COMPILER_VERIF(err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType HandlePop(CmdDataForAsm* CmdInfo, CompilerErrorType* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     char* buffer = GetNextCmd(CmdInfo);
//     size_t BufferLen = strlen(buffer);

//     PopType Pop = {};

//     if (IsRegister(buffer, BufferLen))
//     {
//         PopTypeCtor(&Pop, 1, 0);
//         SetCmdArrCodeElem(CmdInfo, pop);
//         SetCmdArrCodeElem(CmdInfo, GetPopArg(&Pop));
//         SetCmdArrCodeElem(CmdInfo, buffer[0] - 'a');
//     }

//     else if (IsMemory(buffer, BufferLen))
//     {
//         COMPILER_RETURN_IF_ERR(UpdateBufferForMemory(&buffer, &BufferLen);)
//         char* EndBuffer = nullptr;
//         StackElem_t PopElemMemIndex = (int) strtol(buffer, &EndBuffer,  10);

//         if (IsInt(buffer, EndBuffer, BufferLen))
//         {
//             PopTypeCtor(&Pop, 0, 1);
//             SetCmdArrCodeElem(CmdInfo, pop);
//             SetCmdArrCodeElem(CmdInfo, GetPopArg(&Pop));
//             SetCmdArrCodeElem(CmdInfo, PopElemMemIndex);
//         }

//         else if (IsRegister(buffer, BufferLen))
//         {
//             PopTypeCtor(&Pop, 1, 1);
//             SetCmdArrCodeElem(CmdInfo, pop);
//             SetCmdArrCodeElem(CmdInfo, GetPopArg(&Pop));
//             SetCmdArrCodeElem(CmdInfo, buffer[0] - 'a');
//         }
//     }

//     else
//     {
//         err->InvalidInputAfterPop = 1;
//         err->IsFatalError = 1;
//         FREE(buffer);
//         return COMPILER_VERIF(*err);
//     }

//     CmdInfo->FileCmdQuant += 3;

//     return COMPILER_VERIF(*err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType HandleJmp(CmdDataForAsm* CmdInfo, CompilerErrorType* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     JmpCmdPattern(CmdInfo, jmp, err);
//     return COMPILER_VERIF(*err);  
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType HandleJa(CmdDataForAsm* CmdInfo, CompilerErrorType* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     JmpCmdPattern(CmdInfo, ja, err);
//     return COMPILER_VERIF(*err);  
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType HandleJae(CmdDataForAsm* CmdInfo, CompilerErrorType* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     JmpCmdPattern(CmdInfo, jae, err);
//     return COMPILER_VERIF(*err);    
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType HandleJb(CmdDataForAsm* CmdInfo, CompilerErrorType* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     JmpCmdPattern(CmdInfo, jb, err);
//     return COMPILER_VERIF(*err);    
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType HandleJbe(CmdDataForAsm* CmdInfo, CompilerErrorType* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     JmpCmdPattern(CmdInfo, jbe, err);
//     return COMPILER_VERIF(*err);    
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType HandleJe(CmdDataForAsm* CmdInfo, CompilerErrorType* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     JmpCmdPattern(CmdInfo, je, err);
//     return COMPILER_VERIF(*err);  
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType HandleJne(CmdDataForAsm* CmdInfo, CompilerErrorType* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     JmpCmdPattern(CmdInfo, jne, err);
//     return COMPILER_VERIF(*err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType HandleAdd(CmdDataForAsm* CmdInfo, CompilerErrorType* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     NullArgCmdPattern(CmdInfo, add);
//     return COMPILER_VERIF(*err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType HandleSub(CmdDataForAsm* CmdInfo, CompilerErrorType* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     NullArgCmdPattern(CmdInfo, sub);
//     return COMPILER_VERIF(*err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType HandleMul(CmdDataForAsm* CmdInfo, CompilerErrorType* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     NullArgCmdPattern(CmdInfo, mul);
//     return COMPILER_VERIF(*err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType HandleDiv(CmdDataForAsm* CmdInfo, CompilerErrorType* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     NullArgCmdPattern(CmdInfo, dive);
//     return COMPILER_VERIF(*err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType HandleOut(CmdDataForAsm* CmdInfo, CompilerErrorType* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     NullArgCmdPattern(CmdInfo, out);
//     return COMPILER_VERIF(*err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType HandleHlt(CmdDataForAsm* CmdInfo, CompilerErrorType* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     NullArgCmdPattern(CmdInfo, hlt);
//     return COMPILER_VERIF(*err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType NullArgCmdPattern(CmdDataForAsm* CmdInfo, Cmd Cmd)
// {
//     assert(CmdInfo);

//     CompilerErrorType err = {};
//     SetCmdArrCodeElem(CmdInfo, Cmd);
//     CmdInfo->FileCmdQuant++;
//     return COMPILER_VERIF(err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType JmpCmdPattern(CmdDataForAsm* CmdInfo, Cmd JumpType, CompilerErrorType* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     char* JumpArg = GetNextCmd(CmdInfo);
//     CmdInfo->FileCmdQuant += 2;

//     char*  JumpArgEndPtr = nullptr;
//     int    IntJumpArg    = (int) strtol(JumpArg, &JumpArgEndPtr, 10);
//     size_t JumpArgLen    = strlen(JumpArg);
//     int    JumpArgNumLen = (int) (JumpArgEndPtr - JumpArg);

//     if (JumpArgNumLen == (int) JumpArgLen)
//     {
//         SetCmdArrCodeElem(CmdInfo, JumpType);
//         SetCmdArrCodeElem(CmdInfo, IntJumpArg);

//         return COMPILER_VERIF(*err);
//     }

//     int JumpArgInLabeles = IsLabelInLabels(&CmdInfo->Labels, JumpArg);

//     if (JumpArgInLabeles == -1)
//     {
//         Label Temp = {};
//         LabelCtor(&Temp, JumpArg, -1);
//         COMPILER_RETURN_IF_ERR(PushLabel(&CmdInfo->Labels, &Temp));
//         SetCmdArrCodeElem(CmdInfo, JumpType);
//         SetCmdArrCodeElem(CmdInfo, -1);

//         LabelDtor(&Temp);   
//         return COMPILER_VERIF(*err);
//     }

//     int LabelCodePlace = GetLabelCodePlace(CmdInfo, JumpArgInLabeles);
//     SetCmdArrCodeElem(CmdInfo, JumpType);
//     SetCmdArrCodeElem(CmdInfo, LabelCodePlace);

//     return COMPILER_VERIF(*err);
// }

// static int GetLabelCodePlace(CmdDataForAsm* CmdInfo, int Labels_i)
// {
//     assert(CmdInfo);

//     return CmdInfo->Labels.Labels[Labels_i].CodePlace;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType PushTypeCtor(PushType* Push, uint8_t Stk, uint8_t Reg, uint8_t Mem)
// {
//     assert(Push);

//     CompilerErrorType err = {};
//     Push->stk = Stk ? 1 : 0;
//     Push->reg = Reg ? 1 : 0;
//     Push->mem = Mem ? 1 : 0;
//     return err;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static int GetPushArg(PushType* Push)
// {
//     assert(Push);

//     return *(int*) Push;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType PopTypeCtor(PopType* Pop, uint8_t Reg, uint8_t Mem)
// {
//     assert(Pop);
//     CompilerErrorType err = {};
//     Pop->reg = Reg ? 1 : 0;
//     Pop->mem = Mem ? 1 : 0;
//     return err;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static int GetPopArg(PopType* Pop)
// {
//     assert(Pop);
//     return *(int*) Pop;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static size_t CalcFileLen(const char* FileName)
// {
//     assert(FileName);
//     struct stat Buf = {};
//     stat(FileName, &Buf);
//     return (size_t) Buf.st_size;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static void CloseFiles(FILE* ProgrammFilePtr, FILE* CodeFilePtr)
// {
//     assert(ProgrammFilePtr);
//     assert(CodeFilePtr);

//     fclose(ProgrammFilePtr);
//     fclose(CodeFilePtr);
//     return;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static void LabelCtor(Label* Lab, const char* Name, int CodePlace)
// {
//     assert(Lab);
//     assert(Name);

//     Lab->CodePlace = CodePlace;
//     Lab->Name      = strdup(Name);
//     return;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static void LabelDtor(Label* Lab)
// {
//     assert(Lab);

//     Lab->CodePlace = -1;
//     FREE(Lab->Name);
//     return;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType LabelsTableCtor(LabelsTable* Labels)
// {
//     assert(Labels);

//     CompilerErrorType err = {};
//     static const size_t DefaultLabelsTableSize = 16;
//     Labels->FirstFree = 0;
//     Labels->Capacity  = DefaultLabelsTableSize;
//     Labels->Labels    = (Label*) calloc(DefaultLabelsTableSize, sizeof(Label));

//     if (Labels->Labels == nullptr)
//     {
//         err.LabelCallocNull = 1;
//         err.IsFatalError = 1;
//         return COMPILER_VERIF(err);
//     }

//     for (size_t Labels_i = 0; Labels_i < DefaultLabelsTableSize; Labels_i++)
//     {
//         LabelCtor(&Labels->Labels[Labels_i], "", -1);
//     }

//     return COMPILER_VERIF(err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType LabelsTableDtor(LabelsTable* Labels)
// {
//     assert(Labels);

//     CompilerErrorType err = {};
//     Labels->FirstFree = 0;
//     Labels->Capacity  = 0;

//     for (size_t Labels_i = 0; Labels_i < Labels->Capacity; Labels_i++)
//     {
//         LabelDtor(&Labels->Labels[Labels_i]);
//     }

//     FREE(Labels->Labels);

//     return COMPILER_VERIF(err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType PushLabel(LabelsTable* Labels, const Label* Lab)
// {
//     assert(Labels);
//     assert(Lab);

//     CompilerErrorType err = {};

//     if (Labels->FirstFree == Labels->Capacity)
//     {
//         err.TooManyLabels = 1;
//         err.IsFatalError = 1;
//         return COMPILER_VERIF(err);
//     }


//     if (Lab->Name[strlen(Lab->Name) - 1] != ':')
//     {
//         err.PushNotLabel = 1;
//         err.IsFatalError = 1;
//         return COMPILER_VERIF(err);
//     }

//     LabelCtor(&Labels->Labels[Labels->FirstFree], Lab->Name, Lab->CodePlace);

//     Labels->FirstFree++;

//     return COMPILER_VERIF(err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// //возвращает индекс первой встречи, иначе ммнус -1 
// static int IsLabelInLabels(const LabelsTable* Labels, const char* LabelName)
// {
//     assert(Labels);
//     assert(LabelName);

//     for (int Label_i = 0; Label_i < (int) Labels->FirstFree; Label_i++)
//     {
//         if (strcmp(Labels->Labels[Label_i].Name, LabelName) == 0)
//         {
//             return Label_i; 
//         }
//     }
//     return -1;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType FindLabels(CmdDataForAsm* CmdInfo)
// {
//     assert(CmdInfo);

//     CompilerErrorType err = {};
//     for (size_t CmdFuncArr_i = 0; CmdFuncArr_i < CmdFuncQuant; CmdFuncArr_i++)
//     {
//         CmdInfo->Cmd = GetCmdName(CmdFuncArr_i);
//         if (IsLabel(CmdInfo->Cmd))
//         {
//             COMPILER_RETURN_IF_ERR(HandleLabel(CmdInfo, &err));
//             break;
//         }
//     }
//     return COMPILER_VERIF(err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType CmdInfoCtor(CmdDataForAsm* CmdInfo, LabelsTable* Labels, FILE* ProgrammFilePtr, const char* ProgrammFileName, FILE* CodeFilePtr)
// {
//     assert(CmdInfo);
//     assert(Labels);
//     assert(ProgrammFilePtr);
//     assert(ProgrammFileName);
//     assert(CodeFilePtr);

//     CompilerErrorType err = {};

//     CmdInfo->Labels             = *Labels;
//     CmdInfo->ProgrammFilePtr    = ProgrammFilePtr;
//     CmdInfo->ProgrammFileName   = ProgrammFileName;
//     CmdInfo->CodeFilePtr        = CodeFilePtr;
//     CmdInfo->Cmd                = nullptr;
//     CmdInfo->CmdArr             = nullptr;
//     CmdInfo->CmdCodeArr         = nullptr;
//     CmdInfo->FileCmdQuant       = 0;
//     CmdInfo->ProgrammFileLen    = 0;
//     CmdInfo->ProgrammCmdQuant   = 0;
//     CmdInfo->Cmd_i              = 0;
//     CmdInfo->CmdCodeArr_i       = 0;

//     return COMPILER_VERIF(err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType CmdInfoDtor(CmdDataForAsm* CmdInfo)
// {
//     assert(CmdInfo);

//     CompilerErrorType err = {};
//     FREE(CmdInfo->CmdArr);
//     FREE(CmdInfo->CmdCodeArr);
//     // LabelsTableDtor(&CmdInfo->Labels);
//     return COMPILER_VERIF(err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType ReadCmdFromFile(CmdDataForAsm* Cmd)
// {
//     assert(Cmd);

//     CompilerErrorType err = {};

//     size_t BufferLen = CalcFileLen(Cmd->ProgrammFileName);

//     char* buffer     = (char*)  calloc(BufferLen + 1, sizeof(char));
//     Cmd->CmdArr      = (char**) calloc(BufferLen + 1, sizeof(char*));

//     if (buffer == nullptr)
//     {
//         err.FailedAllocateMemoryBufferTempFile = 1;
//         err.IsFatalError = 1;
//         return COMPILER_VERIF(err);
//     }

//     Cmd->ProgrammFileLen = BufferLen;

//     fread(buffer, sizeof(char), BufferLen, Cmd->ProgrammFilePtr);
//     buffer[BufferLen] = '\0';

//     char Temp = GetBufferElem(buffer, 0);
//     if (Temp == ' ' || Temp == '\n')
//     {
//         err.SyntaxisError = 1;
//         err.IsFatalError = 1;
//         return COMPILER_VERIF(err);
//     }

//     Temp = GetBufferElem(buffer, BufferLen - 1);
//     if (Temp != '\n')
//     {
//         err.SyntaxisError = 1;
//         err.IsFatalError = 1;
//         return COMPILER_VERIF(err);
//     }


//     size_t CmdArr_i = 0;
//     SetCmdArr(Cmd, &CmdArr_i, GetBufferElemPtr(buffer, 0));

//     for (size_t buffer_i = 0; buffer_i <= BufferLen; buffer_i++)
//     {
//         Temp = GetBufferElem(buffer, buffer_i);
//         if (Temp == '\n' || Temp == ' ')
//         {
//             while ((Temp == '\n' || Temp == ' ') && buffer_i <= BufferLen)
//             {
//                 buffer[buffer_i] = '\0';
//                 buffer_i++;
//                 Temp = GetBufferElem(buffer, buffer_i);
//             }
//             SetCmdArr(Cmd, &CmdArr_i, GetBufferElemPtr(buffer, buffer_i));
//         }
//     }

//     SetProgrammCmdQuant(Cmd, CmdArr_i - 1);
//     Cmd->CmdArr = (char**) realloc(Cmd->CmdArr, Cmd->ProgrammCmdQuant * sizeof(char*));

//     if (Cmd->CmdArr == nullptr)
//     {
//         err.FailedReallocateMemoryToCmdCodeArr = 1;
//         err.IsFatalError = 1;
//         return COMPILER_VERIF(err);
//     }

//     for (size_t i = 0; i < Cmd->ProgrammCmdQuant; i++)
//     {
//         printf("%s\n", Cmd->CmdArr[i]);
//     }

//     return COMPILER_VERIF(err);
// }


// static CompilerErrorType WriteCmdCodeArrInFile(CmdDataForAsm* Cmd)
// {
//     assert(Cmd);

//     CompilerErrorType err = {};

//     fprintf(Cmd->CodeFilePtr, "%d\n", Cmd->FileCmdQuant);

//     if (fwrite(Cmd->CmdCodeArr, sizeof(int), (size_t) Cmd->FileCmdQuant, Cmd->CodeFilePtr) != (size_t) Cmd->FileCmdQuant)
//     {
//         err.IsFatalError = 1;
//         return COMPILER_VERIF(err);
//     }
//     return COMPILER_VERIF(err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static char GetBufferElem(char* buffer, size_t buffer_i)
// {
//     assert(buffer);

//     return buffer[buffer_i];
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static char* GetBufferElemPtr(char* buffer, size_t buffer_i)
// {
//     assert(buffer);

//     return &buffer[buffer_i];
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static void SetCmdArr(CmdDataForAsm* Cmd, size_t* CmdArr_i, char* SetElem)
// {
//     assert(Cmd);
//     assert(CmdArr_i);
//     assert(SetElem);

//     Cmd->CmdArr[*CmdArr_i] = SetElem;
//     (*CmdArr_i)++;
//     return;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static char* GetNextCmd(CmdDataForAsm* Cmd)
// {
//     assert(Cmd);

//     Cmd->Cmd_i++;
//     return Cmd->CmdArr[Cmd->Cmd_i - 1];
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static void SetProgrammCmdQuant(CmdDataForAsm* Cmd, size_t SetElem)
// {
//     assert(Cmd);
//     Cmd->ProgrammCmdQuant = SetElem;
//     return;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static void SetCmdArrCodeElem(CmdDataForAsm* Cmd, int SetElem)
// {
//     assert(Cmd);
//     Cmd->CmdCodeArr[Cmd->CmdCodeArr_i] = SetElem;
//     Cmd->CmdCodeArr_i++;
//     return;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// void CompilerAssertPrint(CompilerErrorType* err, const char* file, int line, const char* func)
// {
//     assert(err);
//     assert(file);
//     assert(func);

//     COLOR_PRINT(RED, "Assert made in:\n");
//     PrintPlace(file, line, func);
//     PrintError(err);
//     PrintPlace(err->Place.file, err->Place.line, err->Place.func);
//     printf("\n");
//     return;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static CompilerErrorType Verif(CompilerErrorType* err, const char* file, int line, const char* func)
// {
//     assert(err);
//     assert(file);
//     assert(func);

//     ErrPlaceCtor(err, file, line, func);
//     return *err;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static void PrintError(CompilerErrorType* err)
// {
//     assert(err);

//     if (err->IsFatalError == 0)
//     {
//         return;
//     }

//     if (err->InvalidCmd == 1)
//     {
//         COLOR_PRINT(RED, "Error: Invalid commamd.\n");
//     }

//     if (err->SyntaxisError == 1)
//     {
//         COLOR_PRINT(RED, "Error: Invalid syntaxis.\n");
//     }

//     if (err->InvalidInputAfterPop == 1)
//     {
//         COLOR_PRINT(RED, "Error: No/Invalid input after pop.\n");
//     }

//     if (err->NoHalt == 1)
//     {
//         COLOR_PRINT(RED, "Error: No 'hlt'  command.\n");
//     }

//     if (err->FailedAllocateMemoryBufferTempFile == 1)
//     {
//         COLOR_PRINT(RED, "Error: Failed allocalet memory for buffer\nfor TempFile.\n");
//     }

//     if (err->FailedOpenCodeFile == 1)
//     {
//         COLOR_PRINT(RED, "Error: Failed open file with code.\n");
//     }

//     if (err->FailedOpenProgrammFile == 1)
//     {
//         COLOR_PRINT(RED, "Error: Failed open programm file.\n");
//     }

//     if (err->FailedOpenTempFileRead == 1)
//     {
//         COLOR_PRINT(RED, "Error: Failed open TempFile for reading.\n");
//     }

//     if (err->FailedOpenTempFileWrite == 1)
//     {
//         COLOR_PRINT(RED, "Error: Failed open TempFile for wirting.\n");
//     }

//     if (err->NoIntAfterJmp == 1)
//     {
//         COLOR_PRINT(RED, "Error: No number after jmp.\n");
//     }

//     if (err->InvalidInputAfterPush == 1)
//     {
//         COLOR_PRINT(RED, "Error: No/invalid input after push.\n");
//     }

//     if (err->MoreOneEqualLables == 1)
//     {
//         COLOR_PRINT(RED, "Error: 2 equal labels is used.\n");
//     }

//     if (err->PushNotLabel == 1)
//     {
//         COLOR_PRINT(RED, "Error: In labels array was pushed not label.\n");
//     }

//     if (err->FailedReallocateMemoryToCmdCodeArr == 1)
//     {
//         COLOR_PRINT(RED, "Error: Failed to realloc memory for cmd.\n");  
//     }
//     return;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static void ErrPlaceCtor(CompilerErrorType* err, const char* file, int line, const char* func)
// {
//     err->Place.file = file;
//     err->Place.line = line;
//     err->Place.func = func;
//     return;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// static void PrintPlace(const char* file, int line, const char* func)
// {
//     COLOR_PRINT(WHITE, "file [%s]\n", file);
//     COLOR_PRINT(WHITE, "line [%d]\n", line);
//     COLOR_PRINT(WHITE, "func [%s]\n", func);
//     return;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------

// // void CompilerDump(CmdDataForAsm* Cmd, const char* file, int line, const char* func)
// // {
// //     COLOR_PRINT(GREEN, "DUMP BEGIN\n");

// //     COLOR_PRINT(CYAN, "Cmd quant = %d\n\n", Cmd->FileCmdQuant);

// //     for (size_t Labels_i = 0; Labels_i < Cmd->Labels->FirstFree; Labels_i++)
// //     {
// //         COLOR_PRINT(VIOLET, "[%2u]\nName = %s\nCdpl = %d\n", Cmd->Labels->)
// //     }

// //     COLOR_PRINT(GREEN, "DUMP END\n");
// //     return;
// // }
