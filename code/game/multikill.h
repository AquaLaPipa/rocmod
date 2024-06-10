// Copyright (C) 2007 Christian V. Schmidt AKA Commando.DK
//
// Multikill structure

#define MULTIKILL_MAX 6		// Commando.DK, 2008-11-06

typedef struct multikill_s
{
	unsigned char	soundIndex;
	unsigned char	monsterkillEffect;
	int		queueTime[8];
	unsigned char	queue[2][8];
	unsigned char	queueEnd;
	unsigned char	queuePointer;
	unsigned char	kills[MAX_CLIENTS];
	int		first[MAX_CLIENTS];
	int		playNext;
	int		playEnds;
	unsigned char	top;
} multikill_t;

extern	multikill_t	multikill;
