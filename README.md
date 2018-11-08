# MyAI

basically follow the concept: https://luckytoilet.wordpress.com/2012/12/23/2125/



stage1: Uncover the neighbor tiles of "zero" tile first

stage2: Uncover/Flag the easy boundary tiles (can be easly inferred by one edge tile)

stage3: Do edge tiles segmentation and caculate all the configurations. Check if have any 100% safe or 100% mine neighbor tile

stage4: Make the best guess by the probability of mine



current performance

- easy: 832/1000
- medium: 792/1000
- expert: 296/1000



after considering unexplored area

- easy: 862/1000
- medium: 718/1000
- expert: 350/1000



What 's next to do: 

- ~~edge tile segmentation (the optimization mentioned in the link above)~~
- close game strategy (mentioned in the link above)
- hueristic function (?)