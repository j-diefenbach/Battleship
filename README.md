# Battleship
Exploring battleship tactics, and different algorithms for the best gameplay

#### Random Targeting
Random targeting methods take on average, 96 turns to win with a median of 97, sd of 4.5.
  This means its pretty terrible, as expected
  
#### Checkerboard patterns
Since the smallest ship is 2 units long, we don't have to fire on all tiles in order to find all the ships. Using an odd checkerboard pattern we can cut down some of the time it takes to find ships.

#### Probability searching
We could also utilise probability to weigh up all the possible places for ships. Since the smallest ship still sticks out 1 unit from the corners, and this applies to all the ships we can just add up all the possible locations the ships could be. There are much less ways for a ship to fit into the 1 unit corner, then there are for a piece of a ship to be sticking out in the center. So we can apply and then even update these probabilities when seeking ships.

There was a point where I realised I had left a mistake in the algorithm, resulting in some illegal positions considered. This had the unintended effect of causing the algorithm to consider some positions more, leading it to actually perform slightly better in a direct comparison.

![Before_after_prob_fix](https://user-images.githubusercontent.com/105332964/212531510-b1a77027-800e-4981-9319-12d985a567fd.png)

This may be investigated further, as the wider seeking is an aspect of Thompson sampling, meaning that until we have samples an area enough it still holds an equal, or higher, chance of being 'more valuable'. This fix will be kept still, as it likely improves the performance of some aspects and we will revisit the Thompson sampling / exploration-focused integration.
  
## Seeking and hunting
Normal human players will shoot somewhat randomly until they hit a ship. When that happens they start searching around for the rest of the ship. In algorithmic form this is a *finite state machine*, swapping between seeking a ship and hunting that ship until they are confident it is sunk.

#### 4 directional hunting
The smallest ship, the patrol boat is 2 units long, so no matter where we fire, we can expect there to be another hit in at minimum one of the four surrounding squares.
So, using this system the algorithm hunts around all hit squares until it misses sufficient enough times to be sure there are no more ships to be hit. What I did for fun, was see how much difference it makes between the number of consecutive misses and the performance of the tactic.

![Consecutive_misses_to_return_to_seek](https://user-images.githubusercontent.com/105332964/212461463-7ff62458-6f88-4cc6-b867-6a6ee5f3c1dd.png)

As you can see, 3-5 misses is a fairly optimal number, having more than 4 misses means that the algorithm will start going over any spots it missed before (otherwise, it returns to seek). Noise arises from doing only 1000 games on each variable, so to determine between using a factor of 4 or 5 consecutive misses as the return-to-seek point I ran 3000 games on each.
![misses_to_return_to_seek_4_vs_5](https://user-images.githubusercontent.com/105332964/212461625-bb87d9cf-5f1b-42d1-a2f2-31e09f4015c1.png)

In this case line 1 - using a factor of 4 performs slightly better. And using this 4 directional hunting we can show just how much better a checkerboard pattern is when looking for ships, as opposed to random targeting.

![rand_v_checkerboard_4dir_combos](https://user-images.githubusercontent.com/105332964/212461748-eb083f36-3b1b-46f9-832a-44d3b7015bdd.png)

The red line (line number 1) is the first method we looked at, complete randomness is terrible for battleship. When paired with 4 directional hunting it does much better (line 2), however in comparison with utilising checkerboard searching (line 3) the algorithm does even better. The checkerboard pattern is better for finding those first and last few ships.

### Probability applications to hunting

Similar to the 4 directional method, we know that if we hit one square, since the smallest ship is the patrol boat, that there should be at least one more hit in the 4 surrounding squares. The likelihood that there is a hit in the square 2 units away in the 4 directions drops off because we are less sure and so on. We can utilise the probability searching method that we used for seeking, however only on the squares that have been hit. We can calculate all the possible ways a ship can fit that overlaps with the observed hit and also is possible in the board configuration (board size, and misses).

Using this, we can target the square with the highest chance of containing a ship, using the information on our current hits and misses. This far outperforms any of the previous methods, as it is in essence a smarter application of 4 directional hunting.

## Confirming sinks

Under some rules of battleship, players tell each other "you sunk my battleship", this might vary in terms of whether they tell each other what type of ship or even if they tell them they sunk a ship at all. Our algorithm is running off information purely based on the hits and misses.
We can utilise this information to narrow down what ships we've sunked. We know that if we completely surround a 1x2 square with misses and land a hit in there, then it must be the patrol boat. Once we sink this 1x2 square we don't have to look for a patrol boat anymore, and if we find another 1x2 square we know it must be a different kind of ship.

Using a simple algorithm to check for these types of situations, we can 1. Give a much higher weighting to any position we are sure contains an entire ship (a 1x2 square surrounded by misses), and 2. stop considering a ship we are sure we've sunk. This applies best to the probabilistic seeking and hunting method, as we can remove the weighting of the patrol boat for example entirely, which means we don't have to consider the smaller gaps as significantly. The additional weighting for finishing a ship off will likely make our algorithm perform better, as it is given more information once it does so, and it currently considers all hits equal.

## The Human Factor
As we saw before, obviously humans play very differently to random shooting. They also place their ships in a way that is anything but random. Until this point our data has been based off of randomly generated placements of ships. This does pretty well to prove our comparison between different methods of targeting. However, what is theoretically best is often not true in reality, for example people might tend to place their ships closer to the corners.
