s/^Host also known as/Также известен как/
s/^[Ll]ocated in/Расположение:/
s/^[Ll]ocated at the/Расположение:/
s/^[Ll]ocated at/Расположение:/
s/^maintained by/Сервер поддерживает(ют):/
s/^protocols/Протоколы/
s/updated every \(..*\) hours/Обновляется каждые \1 часа(ов)/
s/updated every \(..*\)h/Обновляется каждые \1 часа(ов)/
s/Only reachable from Ukraine/Доступен только из Украины/
s/ from / с /
s/ and / и /
s/and$/и/
s/^and /и /

/^Протоколы/ {
	s/ssh only/только ssh/
	s/ssh port 2022/ssh порт 2022/
}

s/[Cc]entral USA/центральный США/
s/[Ee]astern USA/восточный США/
s/[Ww]estern USA/западный США/
