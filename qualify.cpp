//*****************************************************************
//*  qualify() modifies a string as required to generate a         
//*  "fully-qualified" filename, which is a filename that          
//*  complete drive specifier and path name.                       
//*                                                                
//*  input:  argptr: the input filename.                           
//*                                                                
//*  output: qresult, a bit-mapped unsigned int with the           
//*                   following definitions:                       
//*                                                                
//*          bit 0 == 1 if wildcards are present.                  
//*          bit 1 == 1 if path does not exist.   
//*          bit 2 == 1 if no wildcards and path exists as a file. 
//*          bit 7 == 1 if specified drive is invalid.             
//*                                                                
//*****************************************************************
//  try to modify this function to use string class, vs string.h
//  NOTE that the re-written version of this utility
//  will be Unicode-only, as all my file-handling programs
//  will be Unicode now.
//*****************************************************************

#define  STANDALONE

#include <windows.h>
#include <stdio.h>
#include <direct.h>     //  _getdrive()
#include <sys/stat.h>
#include <shlwapi.h>    // PathIsUNC(), etc
#include <limits.h>
// #include <tchar.h>
#include <vector>
#include <string>
#include <memory> //  unique_ptr

#include "common.h"
#ifndef _lint
#include "conio_min.h"
#endif
// #include "qualify.h"
#define  QUAL_WILDCARDS    0x01
#define  QUAL_NO_PATH      0x02
#define  QUAL_IS_FILE      0x04
#define  QUAL_INV_DRIVE    0x80

#ifndef UNICODE
#error This program can only be used with UNICODE
#include <stophere>
#endif

//lint -e129  declaration expected, identifier ignored
static std::unique_ptr<conio_min> console ;

/******************************************************************/
//lint -esym(714, qualify)  Symbol not referenced
//lint -esym(765, qualify)  external could be made static
unsigned qualify (std::wstring& input_path)
{
   static wchar_t path[MAX_PATH_LEN+1];
   wchar_t *pathptr = &path[0];
   // TCHAR *strptr, *srchptr ;
   DWORD plen;
   int result ;
   struct _stat my_stat ;
   unsigned len, qresult = 0;

   if (input_path.empty()) {
      input_path = L"." ;
   }

   //******************************************************
   //  strings in quotes will also foil the DOS routines;
   //  strip out all double quotes
   //******************************************************
   {  //  begin local context
   std::size_t found = input_path.find_first_of(L"\"");
   while (found != std::wstring::npos)
   {
     // str[found]='*';
     input_path.erase(found, 1);
     found = input_path.find_first_of(L"\"");
   }
   // console->dputsf(_T("after quotes removed: [%s]\n"), input_path.c_str());
   }  //  end local context

   //******************************************************
   //  get expanded path (this doesn't support UNC)
   //******************************************************
   plen = GetFullPathName (input_path.c_str(), MAX_PATH_LEN, (LPTSTR) pathptr, NULL);
   if (plen == 0) {
      return QUAL_NO_PATH;
   }

   len = wcslen (pathptr);
   if (len == 3) {
      wcscat (pathptr, L"*");
      qresult |= QUAL_WILDCARDS;
   }
   else {
      //  see if there are wildcards in argument.
      //  If not, see whether path is a directory or a file,
      //  or nothing.  If directory, append wildcard char
      if (wcspbrk (pathptr, L"*?") == NULL) {
         if (*(pathptr + len - 1) == L'\\') {
            len--;
            *(pathptr + len) = 0;
         }

         //  see what kind of file was requested
         //  FindFirstFile doesn't work with UNC paths,
         //  stat() doesn't either
         // handle = FindFirstFile (pathptr, &fffdata);
         if (PathIsUNC(pathptr)) {
            if (PathIsDirectory(pathptr)) {
               wcscpy (pathptr + len, L"\\*");   //lint !e669  possible overrun
               qresult |= QUAL_WILDCARDS; //  wildcards are present.
            } else if (PathFileExists(pathptr)) {
               qresult |= QUAL_IS_FILE;   //  path exists as a normal file.
            } else {
               wcscpy (pathptr + len, L"\\*");   //lint !e669  possible overrun
               qresult |= QUAL_WILDCARDS; //  wildcards are present.
            }
         } 
         //  process drive-oriented (non-UNC) paths
         else {
            result = _wstat(pathptr, &my_stat) ;
            if (result != 0) {
               qresult |= QUAL_INV_DRIVE; //  path does not exist.
            } else if (my_stat.st_mode & S_IFDIR) {
               wcscpy (pathptr + len, L"\\*");   //lint !e669  possible overrun
               qresult |= QUAL_WILDCARDS; //  wildcards are present.
            } else {
               qresult |= QUAL_IS_FILE;   //  path exists as a normal file.
            }
         }
      }
   }

   //**********************************************
   //  copy string back to input, then return
   //**********************************************
   // _tcscpy (argptr, pathptr);
   input_path = pathptr ;
 //exit_point:   //  jump here if input_path is already updated
   return (qresult); //lint !e438  drive
}

#ifdef  STANDALONE
//********************************************************************************
//  this solution is from:
//  https://github.com/coderforlife/mingw-unicode-main/
//********************************************************************************
#if defined(__GNUC__) && defined(_UNICODE)

#ifndef __MSVCRT__
#error Unicode main function requires linking to MSVCRT
#endif

#include <wchar.h>
#include <stdlib.h>

extern int _CRT_glob;
extern 
#ifdef __cplusplus
"C" 
#endif
void __wgetmainargs(int*,wchar_t***,wchar_t***,int,int*);

#ifdef MAIN_USE_ENVP
int wmain(int argc, wchar_t *argv[], wchar_t *envp[]);
#else
int wmain(int argc, wchar_t *argv[]);
#endif

int main() 
{
   wchar_t **enpv, **argv;
   int argc, si = 0;
   __wgetmainargs(&argc, &argv, &enpv, _CRT_glob, &si); // this also creates the global variable __wargv
#ifdef MAIN_USE_ENVP
   return wmain(argc, argv, enpv);
#else
   return wmain(argc, argv);
#endif
}

#endif //defined(__GNUC__) && defined(_UNICODE)

//**********************************************************************************
//lint -esym(765, test_vectors)  external could be made static
static std::vector<std::wstring> test_vectors {
   L"", 
   L".", 
   L"..",
   L"string\"with\"quotes",
};

//**********************************************************************************
//lint -esym(529, file)  Symbol not subsequently referenced

int wmain(int argc, wchar_t *argv[])
{
   console = std::make_unique<conio_min>() ;
   if (!console->init_okay()) {  //lint !e530
      wprintf(L"console init failed\n");
      return 1 ;
   }
   
   for(auto &file : test_vectors)
   {
      auto test_path = file ;
      
      console->dputsf(L"input file spec: %s\n", test_path.c_str());
      unsigned qresult = qualify(test_path) ;
      if (qresult == QUAL_INV_DRIVE  ||  qresult == QUAL_NO_PATH) {
         console->dputsf(L"error: %s: 0x%X\n", test_path.c_str(), qresult);
      }
      else {
         console->dputsf(L"qualified file spec: %s\n", test_path.c_str());
      }
      console->dputs(L"\n");
      // console->dputs(_T("\n"));
   }
   return 0;
}  //lint !e715 !e818 !e533
#endif
