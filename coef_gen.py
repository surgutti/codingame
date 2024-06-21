
cnt = 0
with open("bot.cpp", "r") as file:
    code = "".join(file.readlines())

    for c1 in [50, 60, 60]:
        for c2 in [90, 100, 110]:
            for c3 in [30, 35, 40, 45, 50]:
            # for c3 in [3, 5, 7]:
            #     for c4 in [1, 2]:        
                    coef1 = c1 / 100
                    coef2 = c2 / 100
                    coef3 = c3 / 100
                    # coef4 = c4 / 100
                    
                    # if coef1 + coef2 + coef3 + coef4 < 1:
                    #     continue
                    
                    # if coef1 + coef2 + coef3 + coef4 > 2:
                    #     continue
                        
                    print("sum: ", coef1 + coef2 + coef3)
                    
                    value_code = code.replace("COEFFICIENT1", str(coef1)).replace("COEFFICIENT2", str(coef2)).replace("COEFFICIENT3", str(coef3)) #.replace("COEFFICIENT4", str(coef4))
                        
                    with open("gen_bots/brainV2_" + str(c1) + "-" + str(c2) + "-" + str(c3) + ".cpp", "w") as bot_file:
                        bot_file.write(value_code)
                    
                    cnt += 1
print(cnt)
