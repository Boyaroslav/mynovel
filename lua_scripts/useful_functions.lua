

function fib(n) return n<2 and n or fib(n-1)+fib(n-2) end

function clock_coroutine()
    while true do
        cllast()  -- стираем старое сообщение
        local t = os.date("%H:%M:%S")  -- текущее время
        txt(t)   -- выводим новое
        wait(1)  -- пауза 1 секунда (yield)
    end
end 

function clock_coroutine()
    while true do
        cllast()  -- стираем старое сообщение
        local t = os.date("%H:%M:%S")  -- текущее время
        txt(t)   -- выводим новое
        wait(1)  -- пауза 1 секунда (yield)
    end
end 


