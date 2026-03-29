'''
 * This file is part of CCN (mynovel).
 * Copyright (C) 2026 Iaroslav Bobylev
 * CCN (mynovel) is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * CCN (mynovel) is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CCN (mynovel). If not, see <https://www.gnu.org/licenses/>.
'''


import os
import struct
from pathlib import Path
import subprocess

VERSION = 1


def fnv1a_32(s: str) -> int:
    h = 0x811c9dc5
    for c in s.encode('utf-8'):
        h ^= c
        h = (h * 0x01000193) & 0xFFFFFFFF
    return h


class CCNVLBuilder:
    
    def __init__(self, path="../project1/", outname="../test2.ccnvl", start_scene="script.bin"):
        self.path = Path(path).resolve()
        self.outname = outname
        self.dumper_path = "../dumper" # надо переделать чтобы он и в stdout печатал результат

        self.index_db = []
        self.start_scene = start_scene
        self.scenes = []
        self.resources = []
        self.chunks = {}

    def dump_file(self, txt_file: Path):

        bin_file = txt_file.with_suffix(".bin")

        with open(txt_file, "rb") as inp, open(bin_file, "wb") as out:

            subprocess.run(
                [self.dumper_path],   # эквивалент ./dumper
                stdin=inp,
                stdout=out,
                check=True
            )

        return bin_file
    
    def compile_scripts(self):

        for file in self.path.glob("*.txt"):
            print("compile", file)
            bin_file = self.dump_file(file)
            self.scenes.append(bin_file)

    def scan_resources(self):
        """Находим ресурсы и чанки во всех подпапках"""
        for f in self.path.rglob("*"):
            if f.is_dir() and f.name.startswith("chunk"):
                files = [sub for sub in f.iterdir() if sub.is_file()]
                self.chunks[f.name] = files
            elif f.is_file() and f.suffix.lower() in [".png", ".ogg", ".wav", ".lua", ".jpg", ".opus", ".mp3"]:
                self.resources.append(f)
        print("Resources found:")
        for r in self.resources:
            print("-", r.relative_to(self.path).as_posix())
        for chunk_name, files in self.chunks.items():
            print(f"Chunk {chunk_name}:")
            for f in files:
                print("   ", f.relative_to(self.path).as_posix())


    def build_index(self):
        # Суммарный размер scene data — ресурсы идут ПОСЛЕ них в ccnvl_data
        total_scene_data_size = sum(f.stat().st_size for f in self.scenes)

        offset = total_scene_data_size  # <-- вот это и было ноль, а должно быть это
        for r in self.resources:
            rel_path = r.relative_to(self.path).as_posix()
            size = r.stat().st_size
            h = fnv1a_32(rel_path)
            self.index_db.append((h, offset, size))
            offset += size

        # чанки — тоже плоские офсеты, без битового пакинга
        chunk_base = offset  # продолжаем после ресурсов
        for chunk_name, files in self.chunks.items():
            local_offset = 0
            for f in files:
                key = f.relative_to(self.path).as_posix()
                size = f.stat().st_size
                h = fnv1a_32(key)
                self.index_db.append((h, chunk_base + local_offset, size))
                local_offset += size
            chunk_base += local_offset

    def write_ccnvl(self):
        with open(self.outname, "wb") as out:

            # ----------------
            # MAGIC + VERSION + START_SCENE
            # ----------------
            out.write(b"CCNVL")
            out.write(struct.pack("<B", VERSION))
            out.write(struct.pack("<I", fnv1a_32(self.start_scene)))

            # ----------------
            # INDEX DB
            # ----------------
            out.write(struct.pack("<I", len(self.index_db)))
            for hsh, indx, size in self.index_db:
                out.write(struct.pack("<I", hsh))
                out.write(struct.pack("<I", indx))
                out.write(struct.pack("<I", size))

            # ----------------
            # SCENE INDEX BUILD
            # ----------------
            scene_index = []
            scene_data = []
            offset = 0
            for scene_path in self.scenes:
                data = scene_path.read_bytes()
                size = len(data)
                scene_hash = fnv1a_32(scene_path.resolve().relative_to(self.path).as_posix())
                scene_index.append((scene_hash, offset, size))
                scene_data.append(data)
                offset += size

            # ----------------
            # SCENE INDEX WRITE
            # ----------------
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

            # ----------------
            # RESOURCE DB (теперь после SCENE DATA)
            # ----------------
            for r in self.resources:
                out.write(r.read_bytes())
            for files in self.chunks.values():
                for f in files:
                    out.write(f.read_bytes())

            # ----------------
            # CHUNK DB
            # ----------------
            out.write(struct.pack("<I", len(self.chunks)))
            for chunk_name in self.chunks:
                name = chunk_name.encode("utf-8")
                out.write(struct.pack("<I", len(name)))
                out.write(name)

            # ----------------
            # END
            # ----------------
            out.write(b"END")



if __name__ == "__main__":
    #b = CCNVLBuilder(input("path: "), input("outname: "), input("start scene: "))
    b = CCNVLBuilder()
    b.compile_scripts()
    b.scan_resources()
    b.build_index()

    b.write_ccnvl()