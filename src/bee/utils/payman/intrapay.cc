#include <sys/types.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <time.h>

#include <bee.h>
#include <ipc.h>

#include "global.h"
#include "intrapay.h"
#include "beetalk.h"
#include "userview.h"
#include "login.h"
#include "log.h"


//#include "da.h"


int    lmonths    = 0;
int    lmonthfrom = 0;

int intrapayDialogProc(DIALOG * th, int action, ulong param)
{
   if (action == PA_EPILOGUE)
   {  
      if (param == ID_OK)
      {  
         lmonths     = strtol(((COMBOX*)(th->ctrl[1].mem))->buff, NULL, 10);
         lmonthfrom  = th->ctrl[4].val;
      }
      return param;
   }

   if (action == PA_INITCTRL)
   {  ((COMBOX*)(th->ctrl[1].mem))->step = 1;
      strcpy( ((COMBOX*)(th->ctrl[1].mem))->buff, "1");  
      return 0; 
   }

   return RET_CONT;
}

char   * startlist[]=
{  "января",
   "февраля",
   "марта",
   "апреля",
   "мая",
   "июня",
   "июля",
   "августа",
   "сентября",
   "октября",
   "ноября",
   "декабря",
   "января (НГ)",     // next year
   "февраля (НГ)",
   "марта (НГ)",
   "апреля (НГ)",
   "мая (НГ)",
   "июня (НГ)",
   "июля (НГ)",
   "августа (НГ)",
   "сентября (НГ)",
   "октября (НГ)",
   "ноября (НГ)",
   "декабря (НГ)"
};

char   startbuf[32]="";

COMBOX startComboMem=
{  0,
   0,
   0,
   startbuf,
   1,
   6,
   18,
   { 12,0,startlist }
};


CONTROL intrapayDialogControls[]=
{  
   {  1,1,1,8,
      CS_DISABLED,
      0xAA55,
      GenControl,
      CT_STATIC,
//2345678901234567890
"Оплачено\0",
      0,0,0,0,0
   },
   {  1,10,1,5,
      CS_DEFAULT,
      0xAA55,
      ComboBoxControl,
      CBT_COUNT | CBT_NUMERIC,
      "1\0",
      0,
      2,
      0,
      0,5
   },
   {  1,16,1,8,
      CS_DISABLED,
      0xAA55,
      GenControl,
      CT_STATIC,
//2345678901234567890
"месяцев\0",
      0,0,0,0,0
   },
   {  3,1,1,10,
      CS_DISABLED,
      0xAA55,
      GenControl,
      CT_STATIC,
//2345678901234567890
"начиная с\0",
      0,0,0,0,0
   },
   {  3,12,1,20,
      CS_EXTERN,
      0xAA55,
      ComboBoxControl,
      CBT_SIMPLE | CBT_READONLY,
      "\0",
      0,
      30,
      &startComboMem,
      0,5
   },
   {  5,3,1,24,
      CS_DEFAULT,
      0xAA55,
      GenControl,
      CT_CHECKBOX,
//2345678901234567890
"включить немедленно\0",
      0,0,0,0,0
   },
   {  0,0,0,0,
      0,
      0,
      0,
      0,
      0,
      0,0,0,0,0
   }
};

DIALOG  intrapayDialog=
{  0,0,
   7,34,
   DS_FRAMED | DS_EPILOGED | DS_PROLOGED,
   DA_DEFAULT,
   SI_NEUTRAL,
   "Оплата сети\0",
   intrapayDialogProc,
   0,
   0,
   0,0,
   intrapayDialogControls
};

char   ostrt_buf[16];
char   ostop_buf[16];
char   nstrt_buf[16];
char   nstop_buf[16];

int IntraPayment()
{  int        rc;
   char     * msg;
   time_t     starttime;
   time_t     stoptime;
   time_t     curtime;

   time_t     new_start;
   time_t     new_stop;

   struct tm  stm;


// Trap N/A account
   if (UserView.user->intra_acc < 0)
   {  MessageBox("Операция невозможна\0",
           " Данная услуга не предоставляется \0",
           MB_OK | MB_NEUTRAL);
      return ID_CANCEL;
   }

// Trap Unlimited accounts
   if ((UserView.accstate2.tag & ATAG_UNLIMIT) != 0)
   {  MessageBox("Операция невозможна\0",
           " Операция запрещена на нелимитированом аккаунте \0",
           MB_OK | MB_NEUTRAL);
      return ID_CANCEL;
   }


// Count month-from start point

   starttime = UserView.accstate2.start; 
   stoptime  = UserView.accstate2.stop;
   curtime   = time(NULL);

   memset(&stm, 0, sizeof(stm));  // for safety 

   if ( (starttime != 0 || stoptime != 0)     &&   // if is ON
         starttime < curtime                  &&
        (stoptime > curtime || stoptime == 0))                            
   {   
      if (stoptime != 0) localtime_r(&stoptime, &stm);
      else localtime_r(&curtime, &stm);
   }
   else 
   {  if (starttime <= curtime) localtime_r(&curtime, &stm);
      else localtime_r(&stoptime, &stm);
   }  

   startComboMem.lst.array = startlist + stm.tm_mon;   

// Reset checkbox
   intrapayDialogControls[5].val = 0;      

// Get sum
   rc = intrapayDialog.Dialog(0);

// Trap Cancel
   if (rc != ID_OK) return rc;

// Trap out of range values
   if (lmonths > 12 || lmonths < 1 ||
       lmonthfrom < 0 || lmonthfrom > 11)
   {  MessageBox("Недопустимый аргумент\0",
           " Недопустимые аргументы начисления \0",
           MB_OK | MB_NEUTRAL);
      return ID_CANCEL;
   }

// Count new start/stop values 

   RefreshConsole();

   uprintf("**** Продляем доступ к сети на %d месяцев, начиная с %s%s\n", 
            lmonths, startlist[lmonthfrom+stm.tm_mon],
            intrapayDialogControls[5].val != 0 ? 
            " (немедленное выполнение)":"");

   uprintf("Вычисляем новые значения старт/стоп ...\n");

   stm.tm_mday = 1;
   stm.tm_hour = 0;
   stm.tm_min  = 0;
   stm.tm_sec  = 0;
   
   stm.tm_mon += lmonthfrom;
   if (stm.tm_mon > 11)
   {  stm.tm_mon  -= 12;
      stm.tm_year++;
   }

   new_start = timelocal(&stm);

   stm.tm_mon += lmonths;
   if (stm.tm_mon > 11)
   {  stm.tm_mon  -= 12;
      stm.tm_year++;
   }

   new_stop  = timelocal(&stm);

   if (stoptime >= new_start) new_start = (-1); 

// Old start
   if (starttime > 0) 
   {  localtime_r(&starttime, &stm);
      strftime(ostrt_buf, sizeof(ostrt_buf), "%d.%m.%Y", &stm);
   } 
   else snprintf(ostrt_buf, sizeof(ostrt_buf), "%s", starttime == 0 ? "NULL" : "(skip)"); 

// Old stop
   if (stoptime > 0) 
   {  localtime_r(&stoptime, &stm);
      strftime(ostop_buf, sizeof(ostop_buf), "%d.%m.%Y", &stm);
   } 
   else snprintf(ostop_buf, sizeof(ostop_buf), "%s", stoptime == 0 ? "NULL" : "(skip)"); 

// New start
   if (new_start > 0) 
   {  localtime_r(&new_start, &stm);
      strftime(nstrt_buf, sizeof(nstrt_buf), "%d.%m.%Y", &stm);
   } 
   else snprintf(nstrt_buf, sizeof(nstrt_buf), "%s", new_start == 0 ? "NULL" : "(skip)"); 

// New stop
   if (new_stop > 0) 
   {  localtime_r(&new_stop, &stm);
      strftime(nstop_buf, sizeof(nstop_buf), "%d.%m.%Y", &stm);
   } 
   else snprintf(nstop_buf, sizeof(nstop_buf), "%s", new_stop == 0 ? "NULL" : "(skip)"); 

   uprintf("старый старт - %s\n", ostrt_buf); 
   uprintf("старый стоп  - %s\n", ostop_buf);
   uprintf("новый  старт - %s\n", nstrt_buf);
   uprintf("новый  стоп  - %s\n", nstop_buf);

//   GetKey();

   uprintf("Соединение с билингом ... ");
   refresh();

   rc = bee_enter();
   if (rc < 0)
   {  uprintf("ОШИБКА.\n");
      uprintf("\n***** Нажмите любую клавишу *****\n"); 
      GetKey();
      return ID_CANCEL;
   }
   uprintf("OK\n");

   log_write("intrapay user \"%s\" acc %d months %d from %s by \"%s\"",
              UserView.user->regname,
              UserView.user->intra_acc,
              lmonths,
              new_start > 0 ? nstrt_buf : ostop_buf,    
              *loggeduser == '\0' ? "NOBODY" : loggeduser);

   uprintf("Установка даты остановки (%s) ... ", nstop_buf);
   refresh();
   rc = bee_send("setstop", "%d %s", UserView.user->intra_acc, nstop_buf);
   if (rc < 0)
   {  uprintf("ПРОГРАММНАЯ ОШИБКА.\n");
      uprintf("\n***** Нажмите любую клавишу *****\n"); 
      bee_leave();
      log_write("newstop error acc %d", UserView.user->intra_acc);
      GetKey();
      return ID_CANCEL;
   }

   msg = NULL;
   rc = bee_recv(RET_SUCCESS, &msg, NULL);
   if (rc < 0 || rc > 400)
   {  uprintf("ОШИБКА (%d).\n", rc);
      uprintf("\n***** Нажмите любую клавишу *****\n"); 
      log_write("newstop error acc %d", UserView.user->intra_acc);
      bee_leave();
      GetKey();
      return ID_CANCEL;
   }
   uprintf("OK\n");

   log_write("newstop acc %d value %s old %s",
              UserView.user->intra_acc,
              nstop_buf,
              ostop_buf);   

   if (new_start > 0)
   {  uprintf("Установка даты старта (%s) ... ", nstrt_buf);
      refresh();
      rc = bee_send("setstart", "%d %s", UserView.user->intra_acc, nstrt_buf);
      if (rc < 0)
      {  uprintf("ПРОГРАММНАЯ ОШИБКА.\n");
         uprintf("\n***** Нажмите любую клавишу *****\n"); 
         bee_leave();
         log_write("newstart error acc %d", UserView.user->intra_acc);
         GetKey();
         return ID_CANCEL;
      }
      msg = NULL;
      rc = bee_recv(RET_SUCCESS, &msg, NULL);
      if (rc < 0 || rc > 400)
      {  uprintf("ОШИБКА (%d).\n", rc);
         uprintf("\n***** Нажмите любую клавишу *****\n"); 
         bee_leave();
         log_write("newstart error acc %d", UserView.user->intra_acc);
         GetKey();
         return ID_CANCEL;
      }
      uprintf("OK\n");

      log_write("newstart acc %d value %s old %s",
                 UserView.user->intra_acc,
                 nstrt_buf,
                 ostrt_buf);   

   }
   else uprintf("(Пропуск установки даты старта)\n"); 

   if (intrapayDialogControls[5].val != 0)
   {  if (AccessLevel < AL_MASTER) 
      {  uprintf("Обновление Фаза 1 (эмуляция)... ");   
         refresh();
         sleep(1);
         uprintf("OK\n");
         uprintf("Обновление Фаза 2 (эмуляция)... ");   
         refresh();
         sleep(2);
         uprintf("OK\n");
      }  
      else
      {  uprintf("Обновление Фаза 1 ... ");   
         refresh();

         rc = bee_update();
         if (rc < 0)
         {  uprintf("ПРОГРАММНАЯ ОШИБКА.\n");
            uprintf("\n***** Нажмите любую клавишу *****\n");
            bee_leave();
            GetKey();
            return ID_CANCEL;
         }

         uprintf("OK\n");
         uprintf("Обновление Фаза 2 ... ");   
         refresh();

         rc = bee_intraupdate();
         if (rc < 0)
         {  uprintf("ПРОГРАММНАЯ ОШИБКА.\n");
            uprintf("\n***** Нажмите любую клавишу *****\n");
            bee_leave();
            GetKey();
            return ID_CANCEL;
         }
         uprintf("OK\n");
      }  
           
   }


   uprintf("Отсоединяемся ... ");
   refresh();
   bee_leave();
   uprintf("OK\n");

//   GetKey();

   UserView.load_accs();

   return ID_OK;
}
