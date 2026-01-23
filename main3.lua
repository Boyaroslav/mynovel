-- $Name: Лия и Кок$
-- $Version: 1.0.0$
-- $Author: Ярослав Бобылев$
require "fmt"
require "noinv"
require "snd"

fmt.para = true
global{
main_visited = 0;
egg_looked = 0;
day = 0;
extra = 0;


}
include "episodes/1"
include "episodes/2"
include "episodes/thinks"


room {
    nam = 'main';
    title = 'С чего все началось...';
    disp = 'В гостиной';
    decor = function()
        if day == 0 then
            if main_visited < 1 then
                pn 'Уже который час!';
                pn 'Никогда такого не было, чтобы ваша девушка, Лия, задерживалась так долго. Ваше сердце бешено бешено бьется в страхе за любимую';
                pn 'В растерянности ваши глаза ходят по комнате...';
                pn '{#dvor|Двор} {#spalna|Спальня}';
                main_visited = 1;
            else 
                pn 'Все остается тем же, ничто не меняется';
                pn '{#dvor|Двор} {#spalna|Спальня}';
            end
        end
        if day == 1 then
            pr [[Лия ждала вас весь день, читая конспекты, сделанные ей в пору её учебы в университете МИСИС. ^^
            В отличие от вас, она сумела окончить московский вуз, в то время как вы не можете окончить и провинциального. ^^
            {#think_one|Ну и что} {#spalna|Поздороваться, и пройти в спальню}]]
        end

    end
    --dsc = [[Уже который час!]];
} : with{
    'Телефон';
    think1;
    obj {
        nam = "#dvor";
        act = function ()
            walk 'dvor';
        end
    };
        obj {
        nam = "#spalna";
        act = function ()
            walk 'spalna';
        end
    };
}