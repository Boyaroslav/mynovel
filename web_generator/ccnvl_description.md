

расшареная папка с ресурсами и .json

монтируем - открываем и редактируем каждый свой .json в веб редакторе
на сервере - скрипт - юзает для каждого .json dumper, потом собирает все ресурсы и скрипты и делает файл .ccnvl

MAGIC "CCNVL"

VERSION number (uint8)

INDEX DB - хеш - индекс размер (только для ресурсов - картинки музыка скрипты на луа)
если ресурс в чанке, то даем число - [номер чанка(1 число)][смещение]

struct index_db_element{
	hash
	index
	size
}

SCENE STORAGE
    scene1.bin
    scene2.bin
    final_scene.bin

CHUNK DB - просто строчка название файла чанка в настоящей файловой системе

END



CHUNK

будет генерироваться из расположения ресурсов


примерная схема проекта

picture.png
script.bin
script2.bin
project.json
project2.json
sound.ogg
speech.ogg
talk1.ogg
talk2.ogg
final_part.json
chunk1:
	high_res_picture.png
	music.wav

chunk2:
	8k_picture.png
	final_song.wav


исходя из расположения файлов генератор ccnvl будет генерировать чанки chunk1 и chunk2, и в index db добавлять на хэш их названия hash[chunk1/high_res_picture.png] ссылку в соответствующем формате