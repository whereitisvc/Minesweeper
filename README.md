# MyAI

basically follow the concept: https://luckytoilet.wordpress.com/2012/12/23/2125/



stage1: Uncover the neighbor tiles of "zero" tile first

stage2: Uncover/Flag the easy boundary tiles (can be easly inferred by one edge tile)

stage3: Caculate all the configurations and make the best decision. Check if have any 100% safe or 100% mine neighbor tile

stage4: Make the best guess by the probability of mine



current performance

- easy: 85/100
- medium: 78/100
- expert: 8/20 (will run too long in some case, so I have not tested too much)



What 's next to do: 

- edge tile segamentation (the optimization mentioned in the link above)
- close game strategy (mentioned in the link above)
- hueristic function (?)