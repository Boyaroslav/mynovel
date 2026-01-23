



room {
    nam = 'morning';
    dsc = function ()
        if day == 1 then
            pr [[Новое утро как всегда радовало лишь вашей Лией^^
            Как же она красива по утрам. Но к сожалению вам надо торопиться...^^
            {#work|Пойти на учёбу}]];
        end
    end
} : with{
    obj {
        nam = '#work';
        act=  function()
            walk 'work';
        end
    };
}