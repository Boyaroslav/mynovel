class Scene:
    def __init__(self, id, bg, sprites, actions, nextid, change_effect, intro, effect):
        self.id = id
        self.bg = bg
        self.sprites = sprites
        self.actions = actions
        self.nextid = nextid
        self.intro = intro
        self.change_effect = change_effect
        self.effect = effect
