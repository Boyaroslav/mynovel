import pygame
import os
from display import *

main_base = open("base.cpp", errors="ignore").read()

def create_project(name):
    os.system(f"mkdir {name}")
    if os.name:
        if not os.path.exists(f"{name}\\main.cpp"):
            with open(f"{name}\\main.cpp", 'w') as m:
                m.write(main_base)
        os.system(f"mkdir {name}\\scenes")
        os.system(f"mkdir {name}\\sprites")
        os.system(f"mkdir {name}\\sounds")
        os.system(f"mkdir {name}\\cutscenes")
        os.system(f"mkdir {name}\\effects")


def main():
    t = input('Папка нового проекта: ')
    create_project(t)
    print("Done!")
    m = Window(t)
    m.draw(ROOT)


if __name__ == '__main__':
    main()
