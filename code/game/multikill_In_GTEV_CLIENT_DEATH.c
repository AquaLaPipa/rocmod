// Copyright (C) 2007 Christian V. Schmidt AKA Commando.DK


			//if (attacker->s.number >= 0)		// Attacker is client is checked earlier in g_combat.c. - Commando.DK, 2010-03-23 12:10
			{
				if ( level.time < multikill.first[attacker->s.number] + multikill.kills[attacker->s.number] * g_multikillInterval.integer )
				{
					multikill.kills[attacker->s.number] += 1;
					if ( (multikill.playEnds < level.time || multikill.top <= multikill.kills[attacker->s.number]) && g_multikillMin.integer <= multikill.kills[attacker->s.number] )
					{
						if (multikill.playNext < level.time)
						{
							multikill.playNext = level.time + 100;
						}
						if (multikill.queueEnd == multikill.queuePointer || multikill.queue[0][multikill.queueEnd] != attacker->s.number)
						{
							multikill.queueEnd = (multikill.queueEnd + 1) % (sizeof(multikill.queue) / (sizeof(char) * 2));
							multikill.queue[0][multikill.queueEnd] = attacker->s.number;
						}
						multikill.queue[1][multikill.queueEnd] = multikill.kills[attacker->s.number];
						multikill.queueTime[multikill.queueEnd] = level.time;
					}
				}
				else
				{
					multikill.kills[attacker->s.number] = 1;
					multikill.first[attacker->s.number] = level.time;
				}
			}
			multikill.kills[self->s.number] = 0;
