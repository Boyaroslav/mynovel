

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

	hashes of scene file names

    scene1.bin content
    scene2.bin content
    final_scene.bin content


            out.write(struct.pack("<I", len(scene_index)))

            for hsh, off, size in scene_index:

                out.write(struct.pack("<I", hsh))
                out.write(struct.pack("<I", off))
                out.write(struct.pack("<I", size))

            # ----------------
            # SCENE DATA
            # ----------------

            for data in scene_data:
                out.write(data)




CHUNK DB - просто строчка название файла чанка в настоящей файловой системе

END



CHUNK

будет генерироваться из расположения ресурсов


примерная схема проекта

picture.png
script.bin
script.txt
script2.txt
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

dumper будет применяться к содержимому file.txt, там находятся комманды сгенерированные веб редактором из file.json. Результатом выполнения является file.bin, который уже может исполняться игрой


исходя из расположения файлов генератор ccnvl будет генерировать чанки chunk1 и chunk2, и в index db добавлять на хэш их названия hash[chunk1/high_res_picture.png] ссылку в соответствующем 

получается грузить ресурсы будем лениво и с каждой новой сценой сбрасывать ( ну или 20 сцен хз )