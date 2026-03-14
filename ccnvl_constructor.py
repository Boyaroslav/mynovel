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
    
    def __init__(self, path, outname, start_scene="scene.bin"):
        self.path = Path(path)
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

        for f in self.path.iterdir():

            if f.is_dir() and f.name.startswith("chunk"):

                files = []
                for sub in f.iterdir():
                    if sub.is_file():
                        files.append(sub)

                self.chunks[f.name] = files

            elif f.suffix in [".png", ".ogg", ".wav", ".lua"]:

                self.resources.append(f)

    def build_index(self): # builds index db

        # обычные ресурсы

        offset = 0

        for r in self.resources:

            size = r.stat().st_size
            h = fnv1a_32(r.name)

            self.index_db.append(
                (h, offset, size)
            )

            offset += size

        # chunk ресурсы

        for i, (chunk_name, files) in enumerate(self.chunks.items()):

            offset = 0

            for f in files:

                key = f"{chunk_name}/{f.name}"
                h = fnv1a_32(key)
                size = f.stat().st_size

                index = (i << 24) | offset

                self.index_db.append(
                    (h, index, size)
                )

                offset += size

    def write_ccnvl(self):

        with open(self.outname, "wb") as out:

            # ----------------
            # MAGIC + VERSION
            # + START_SCENE
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

                with open(scene_path, "rb") as f:
                    data = f.read()

                size = len(data)

                scene_hash = fnv1a_32(scene_path.stem)

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
    b = CCNVLBuilder(input("path: "), input("outname: "), input("start scene: "))
    
    b.compile_scripts()
    b.scan_resources()
    b.build_index()

    b.write_ccnvl()