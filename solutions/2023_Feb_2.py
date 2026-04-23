#These should be set by autocheck
#will fail if student tries to override this data in their own program

TeamName = ['A', 'B', 'C', 'D', 'E']
TeamPoints = [[3, 2, 1, 0, 0],
              [1, 1, 1, 0, 0],
              [3, 2, 1, 0, 0],
              [3, 2, 1, 0, 0],
              [3, 3, 3, 3, 3]]

LeagueSize = 5
MatchNo = 5

#start of student code here

totals = [0, 0, 0, 0, 0]

for i in range(LeagueSize):
    name = TeamName[i]
    points = TeamPoints[i]
    away_wins = 0
    home_wins = 0
    draws = 0
    losses = 0
    for j in range(MatchNo):
        p = points[j]
        if p == 3:
            away_wins +=1
        elif p == 2:
            home_wins += 1
        elif p == 1:
            draws += 1
        else:
            losses += 1      
        totals[i] += p
    print("Team:", name, "    ")
    print("Total Points:", totals[i])
    print("Away Wins:", away_wins)
    print("   Home Wins:", home_wins)
    print("Drawn Matches:", draws)
    print("Lost Matches:", losses)
    print("   ")


lowest_team = TeamName[0]
lowest_total = totals[0]
highest_team = TeamName[0]
highest_total = totals[0]
for i in range(LeagueSize):
    t = totals[i]
    if t > highest_total:
        highest_total = t
        highest_team = TeamName[i]
    if t < lowest_total:
        lowest_total = t
        lowest_team = TeamName[i]

print("Highest Points:", highest_team)
print("Lowest Points:", lowest_team)
