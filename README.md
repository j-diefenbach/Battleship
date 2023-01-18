# Battleship
Exploring battleship tactics, and different algorithms for approaching optimal gameplay.

#### Random Targeting
Random targeting methods take 97 turns to win 50% of its games. So in the other 50% of its games the last 3 turns are when it lands its final shots. As should be expected, relying on pure chance does terribly.
  
#### Checkerboard patterns
Since the smallest ship is 2 units long, we don't have to fire on all tiles in order to find all the ships. Using an odd checkerboard pattern we can cut down some of the time it takes to find ships. But in this case, we don't have an approach so sink the ships that don't lie on this checkerboard.

#### Probability searching
We could also utilise probability to weigh up all the possible places for ships. Since the smallest ship still sticks out 1 unit from the corners, and this applies to all the ships we can just add up all the possible locations the ships could be. There are much less ways for a ship to fit into the 1 unit corner, then there are for a piece of a ship to be sticking out in the center. So we can apply and then even update these probabilities when seeking ships.
Again, using this approach is good for finding ships but it doesn't quickly cut down ships like your average player does once they land a hit.

---
There was a point later on where I realised I had left a mistake in the algorithm, resulting in some illegal positions considered. This had the unintended effect of causing the algorithm to consider some less likely positions more likely, leading it to actually perform slightly better in a direct comparison.

This may be investigated further, as the wider seeking is an aspect of Thompson sampling, meaning that the less we have 'sampled' (or fired upon) an area, the higher chance it could be 'more valuable' than the other areas. This fix will be kept still, as it likely improves the performance of some other implementations further down the line, and we will revisit the Thompson sampling / exploration-focused integration.
  
## Seeking and hunting
Normal human players will shoot somewhat randomly until they hit a ship. When that happens they start searching around for the rest of the ship. In algorithmic form this is a *finite state machine*, swapping between looking all over the board to find a ship, and using the hits they find to hunt down and sink that ship. The best approach is generally to swap back and forth, as once a player is confident they've finished off a ship, or need more information, they go back to 'seeking' until they land another hit.

#### 4 directional hunting
The smallest ship, the patrol boat is 2 units long, so no matter where we fire, we can expect there to be another hit in at minimum one of the four surrounding squares.
So, using this system the algorithm hunts around all hit squares until it misses sufficient enough times to be sure there are no more ships to be hit. What I did for fun, was see how much difference it makes between the number of consecutive misses and the effect on performance.

![4dirhunt_adj_factors](https://user-images.githubusercontent.com/105332964/212787567-87302783-4ae9-4d72-b693-cae24c439153.png)


As you can see, 3-5 misses is a fairly optimal number, having more than 4 misses means that the algorithm will start going over any spots it missed before (otherwise, it returns to seek). Noise arises from doing only 1000 games on each variable, so to determine between using a factor of 4 or 5 consecutive misses as the return-to-seek point I ran 3000 games on each. Using a factor of 4 still outperforms the others slightly, so I chose that.

#### Seek mode comparison
![seek_comparison](https://user-images.githubusercontent.com/105332964/212787731-7def521e-ffc6-4711-be0b-399cb7ad8d26.png)

Pure random as a baseline performs terribly. However, you can see just applying 4dir hunting makes it perform far better as the algorithm will finish off any targets it finds. Still using 4dir hunting, seeking using a checkerboard performs slightly better as the maximum number of squares to find all the ships is almost half now.
Probabilistic hunting far outperforms the others though, finishing 90% of games where the checkerboard approach only finishes slightly more than 75%. To compare the medians -  the number of turns it takes for the approach to win 50% of its games:

1. Probability seeking using 4 directional hunting: 60.8 turns
2. Checkerboard seeking using 4 directional hunting: 68 turns
3. Random seeking using 4 directional hunting: 71 turns
4. Pure random: 97 turns

### Probability applications to hunting

Similar to the 4 directional method, we know that if we hit one square, since the smallest ship is the patrol boat, that there should be at least one more hit in the 4 surrounding squares. The likelihood that there is a hit in the square 2 units away in the 4 directions drops off because we are less sure and so on. We can utilise the probability searching method that we used for seeking, however only on the squares that have been hit. We can calculate all the possible ways a ship can fit that overlaps with the observed hit and also is possible in the board configuration (board size, and misses).

Using this, we can target the square with the highest chance of containing a ship, using the information on our current hits and misses. This far outperforms any of the previous methods, as it is in essence a smarter application of 4 directional hunting.

## You Sunk My Battleship
Under the official rules of battleship, a player must tell the other player when they have sunk a ship, including which type of ship. If we incorporate this into our algorithm, we can use it for helping our algorithm narrow down ship positions.

If we know that the patrol boat is sunk, we no longer have to check positions that only the patrol boat can fit in. We can also use an additional algorithm to determine where the patrol boat is that we have sunk, and once that can be determined the tiles of the sunk ship can be considered an obstruction. This allows the list of possible arrangments to be smaller and more accurate.

Using a simple algorithm to check for these types of situations, we can 1. Give a much higher weighting to any position we are sure contains an entire ship (a 1x2 square surrounded by misses), and 2. stop considering a ship we are sure we've sunk. This applies best to the probabilistic seeking and hunting method, as we can remove the weighting of the patrol boat for example entirely, which means we don't have to consider the smaller gaps as significantly. The additional weighting for finishing a ship off will likely make our algorithm perform better, as it is given more information once it does so, and it currently considers all hits equal.

## The Human Factor
As we saw before, obviously humans play very differently to random shooting. They also place their ships in a way that is anything but random. Until this point our data has been based off of randomly generated placements of ships. This does pretty well to prove our comparison between different methods of targeting. However, what is theoretically best is often not true in reality, for example people might tend to place their ships closer to the corners more often than the current probability algorithm predicts.
