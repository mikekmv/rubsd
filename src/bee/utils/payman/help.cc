#include "global.h"

int RefreshHelp()
{
   if (DoRefresh == 0) return 0;

   if (StageScr == 2)
   {  Gotoxy(2, 10); Attr(7, 0);
      uprintf(
"������� ���������� ��� ������ �������������\n"
"\n"
"   Ctrl-L - ������������ ����� (������ �����)\n"
"\n"
"   �������, PgUp, PgDn, Home, End - ����������� �� ������ \n"
"\n"
"   F1  - ������ (�� ������� ������)\n"
"   F2  - ����������� ������ �� �����\n"
"   F3  - ����������� ������ �� IP-������\n"
"   F4  - ����������� ������ �� �����/�����\n"
"   F5  - �������� �������\n"
"   F8  - ������������� ������ �� ����\n"
"   F9  - (����-)���������� ��� ������ �������������\n"
"   F10 - ��������� ������ ���������\n"
"\n"
"   ���������-�������� ������� - ����� �� �����\n"
"\n"
"\n"
"   ��� �������� � ���������� ����� ������� F10 ��� Esc.\n"
"              Esc ����������� � ��������� !\n"
);
   }

   if (StageScr == 3)
   {  Gotoxy(2, 10); Attr(7, 0);
      uprintf(
"������� ���������� ��� ��������� ������������\n"
"\n"
"   Ctrl-L - ������������ ����� (������ �����)\n"
"\n"
"   F1  - ���� ����� ������\n"
"\n"
"   F2  - �������� �� ���������\n"
"   F3  - �������� �� ����\n"
"\n"
"   F10 � Esc - �������� � ������ �������������\n"
"\n"
"   ��� �������� � ���������� ����� ������� F10 ��� Esc.\n"
"               Esc ����������� � ��������� !\n"
"\n"
);
   }

   return 0;
}
