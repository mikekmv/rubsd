s/^Host also known as/����� �������� ���/
s/^[Ll]ocated in/������������:/
s/^[Ll]ocated at the/������������:/
s/^[Ll]ocated at/������������:/
s/^maintained by/������ ������������(��):/
s/^protocols/���������/
s/updated every \(..*\) hours/����������� ������ \1 ����(��)/
s/updated every \(..*\)h/����������� ������ \1 ����(��)/
s/Only reachable from Ukraine/�������� ������ �� �������/
s/ from / � /
s/ and / � /
s/and$/�/
s/^and /� /

/^���������/ {
	s/ssh only/������ ssh/
	s/ssh port 2022/ssh ���� 2022/
}

s/[Cc]entral USA/����������� ���/
s/[Ee]astern USA/��������� ���/
s/[Ww]estern USA/�������� ���/
