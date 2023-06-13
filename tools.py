import os


def load_scene(project, name):
    try:
        f = open(f"{project}\\scenes\\{name}").readlines()
        ans = {
            'Title': '',
            'Bg': None,
            'Sprites': [],
            'ChangeLine': [],
            'Action': [],
            'NextID': None,
            'into': None,
            'change_effect': None,
            'effect': None
        }
        for i in f:
            if "=" in i:
                if type(ans[i[:i.find('=')]]) == type([]):
                    ans[i[:i.find('=')]].append(i[i.find('=') + 1:].strip().split())
                else:
                    ans[i[:i.find('=')]] = i[i.find('=') + 1:].strip().replace('\n', '')

        return ans
    except:
        names = filter(lambda x: x[:-1] == name, os.listdir(f"{project}\\scenes"))
        ans = []
        for i in names:
            ans.append(load_scene(project, i))
        return ans

