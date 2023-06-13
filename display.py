import pygame
import os
from tools import *
pygame.init()

ROOT = None
WINDOW = 1


class Window:
    def __init__(self, project):
        self.project = project
        self.font = pygame.font.SysFont('monospace', 10)
        self.big_font = pygame.font.SysFont('monospace', 100)
        self.scenes = os.listdir(f"{self.project}\\scenes")
        self.cutscenes = os.listdir(f"{self.project}\\cutscenes")
        self.sprites = os.listdir(f"{self.project}\\sprites")
        self.sounds = os.listdir(f"{self.project}\\sounds")
        self.effects = os.listdir(f"{self.project}\\effects")
        self.scale = 5
        self.scenes = self.render_scenes_tree(1)
        #pygame.transform.scale(self.render_scene(1), (1920 / 5, 1080 / 5))
    def render_scenes_tree(self, start):
        ret = []
        while start:
            s = load_scene(self.project, start)
            b = []
            b.append(self.render_scene(start, name=start))
            b.append(s)
            b.append(-1)

            ret.append(b)
            print(ret)
            if type(s) != type([]):
                if s['NextID']:
                    ret[-1][1] = len(ret)
                    start = s['NextID']
                else:
                    start = None
            else:
                if s[0]['NextID']:
                    ret[-1][1] = len(ret)
                    start = s[0]['NextID']
                else:
                    start = None

        return ret




    def render_scene(self, scene, jsinfo=None, name=None):
        if name == None:
            name = scene
        if jsinfo:
            r = jsinfo
        else:
            r = load_scene(self.project, scene)
        if type(r) != type([]):
            surface = pygame.Surface((1920, 1080))
            if r['Bg']:
                bg = pygame.image.load(f"{self.project}\\sprites\\{r['Bg']}")
                surface.blit(bg, (0, 0))

            if r['Sprites']:
                for i in r['Sprites']:
                    n = pygame.image.load(f"{self.project}\\sprites\\{i[-1]}")
                    surface.blit(n, (int(i[0]), int(i[1])))

            surface.blit(self.big_font.render(str(name), True, (255, 0, 0)), (0, 0))
        else:
            surface = []
            for i in r:
                surface.append(self.render_scene('', i, name=name))

        return surface

    def draw(self, root):
        prj = self.font.render(str(self.project), True, (255, 255, 255))
        self.x, self.y = 0, 0
        bx = 0
        by = 0
        lbx, lby = 0, 0
        lastx, lasty = None, None
        if root == None:
            root = pygame.display.set_mode((1000, 800))
        global WINDOW
        while WINDOW:
            for i in pygame.event.get():
                if i.type == pygame.QUIT:
                    WINDOW = 0
                    quit()
                if i.type == pygame.MOUSEBUTTONDOWN:
                    lastx, lasty = pygame.mouse.get_pos()
                if i.type == pygame.MOUSEBUTTONUP:
                    lastx, lasty = None, None
                    lbx = bx
                    lby = by


            if lastx != None and lasty != None:
                m = pygame.mouse.get_pos()
                bx = lastx - m[0] + lbx
                by = lasty - m[1] + lby

            root.fill((50, 50, 50))
            for i in range(0, len(self.scenes)):
                if type(self.scenes[i][0]) != type([]):
                    root.blit(pygame.transform.scale(self.scenes[i][0], (1920 / 5, 1080 / self.scale)), (-bx + i * 1920 * 1.5 / self.scale, -by))
                else:
                    for j in range(0, len(self.scenes[i][0])):
                        root.blit(pygame.transform.scale(self.scenes[i][0][j], (1920 / 5, 1080 / self.scale)),
                                  (-bx + i * 1920 * 1.5 / self.scale, -by - j * self.scale * 50))
            root.blit(prj, (10, 10))
            pygame.display.update()

