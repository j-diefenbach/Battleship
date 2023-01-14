# Battleship
Exploring battleship tactics, and different algorithms for the best gameplay

#### Random Targeting
Random targeting methods take on average, 96 turns to win with a median of 97, sd of 4.5.
  This means its pretty terrible, as expected
  
#### Checkerboard patterns
Since the smallest ship is 2 units long, we don't have to fire on all tiles in order to find all the ships. Using an odd checkerboard pattern we can cut down some of the time it takes to find ships. However, when finding ships we can't continue using a checkerboard pattern to fire upon, instead we need to target ships directly...
  
### Seeking and hunting
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

## The Human Factor
As we saw before, obviously humans play very differently to random shooting. They also place their ships in a way that is anything but random. Until this point our data has been based off of randomly generated placements of ships. This does pretty well to prove our comparison between different methods of targeting. However, what is theoretically best is often not true in reality, for example people might tend to place their ships closer to the corners.
