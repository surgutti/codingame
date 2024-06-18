

with open("bot.cpp", "r") as file:
    code = "".join(file.readlines())
    
    for c1 in range(1, 20, 3):
        for c2 in range(1, 50, 7):
            coef1 = c1 / 10
            coef2 = c2 / 100
            
            value_code = code.replace("COEFFICIENT1", str(coef1)).replace("COEFFICIENT2", str(coef2))
                
            with open("gen_bots/bot_" + str(c1) + "-" + str(c2) + ".cpp", "w") as bot_file:
                bot_file.write(value_code)