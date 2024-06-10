// Copyright (C) 2007 Christian V. Schmidt AKA Commando.DK


	if (multikill.queuePointer != multikill.queueEnd && multikill.playNext < level.time)
	{
		static int const playlength[2][5] = { {420, 595, 650, 700, 590}, {1000, 1320, 1270, 1600, 3800} };

		multikill.queuePointer = (multikill.queuePointer + 1) % (sizeof(multikill.queue) / (sizeof(char) * 2));
		if (multikill.queue[1][multikill.queuePointer] == multikill.kills[multikill.queue[0][multikill.queuePointer]] && (multikill.playEnds < level.time || multikill.top <= multikill.queue[1][multikill.queuePointer]))
		{
			int tempi = multikill.queueTime[multikill.queuePointer] - multikill.first[multikill.queue[0][multikill.queuePointer]];

			//trap_SendServerCommand( -1, va("cp \"^%c%s ^7KILLED ^%c%i ^7IN ^%c%i.%i ^7SECONDS !\n\"", properTeamcolor[g_entities[multikill.queue[0][multikill.queuePointer]].client->sess.team], g_entities[multikill.queue[0][multikill.queuePointer]].client->pers.netname, properTeamcolor[g_entities[multikill.queue[0][multikill.queuePointer]].client->sess.team], multikill.queue[1][multikill.queuePointer], properTeamcolor[g_entities[multikill.queue[0][multikill.queuePointer]].client->sess.team], tempi/1000, (tempi%1000)/100));
			trap_SendServerCommand( -1, va("cp \"^%c%s ^7KILLED ^1%i ^7IN ^1%i.%i ^7SECONDS !\n\"", g_entities[multikill.queue[0][multikill.queuePointer]].client->sess.team-TEAM_RED ? '4' : '1', g_entities[multikill.queue[0][multikill.queuePointer]].client->pers.netname, multikill.queue[1][multikill.queuePointer], tempi/1000, (tempi%1000)/100));
			if ( multikill.queue[1][multikill.queuePointer] < MULTIKILL_MAX )		
			{
				tempi = multikill.queue[1][multikill.queuePointer] - 2;
			}
			else
			{
				static vec3_t up = {0,0,1};

				for ( tempi = 0;  tempi < level.numConnectedClients; tempi++ ) 
				{
					G_PlayEffect ( multikill.monsterkillEffect, (&g_entities[level.sortedClients[tempi]])->r.currentOrigin, up );
				}
				tempi = MULTIKILL_MAX-2;		
			}
			G_BroadcastSoundIndex(multikill.soundIndex + tempi);
			multikill.top = multikill.queue[1][multikill.queuePointer];
			multikill.playNext = level.time + playlength[0][tempi];
			multikill.playEnds = level.time + playlength[1][tempi];
		}
	}
