#pragma once

#include "GuiReflection.h"

struct Settings
{
	int slashFrameDuration = 2;

	struct Enemy {
		float patrolSpeed = 1.0f;
		float chaseSpeed = 1.5f;
		float patrolTurnRate = 0.1f;
		float sightDistance = 5.0f;
		
		struct Attack {
			float prepareDuration = 1.0f;
			float prepareSpeed = -1.0f;
			float attackDuration = 1.0f;
			float attackSpeed = 2.0f;
			float postAttackPauseDuration = 0.5f;
		};
	};

	Enemy critter;
	Enemy::Attack critterAttack;
};

JSONANDGUI_REFLECTION(Settings::Enemy::Attack, prepareDuration, prepareSpeed, attackDuration, attackSpeed, postAttackPauseDuration);
JSONANDGUI_REFLECTION(Settings::Enemy, patrolSpeed, chaseSpeed, patrolTurnRate, sightDistance);
JSONANDGUI_REFLECTION(Settings, slashFrameDuration, critter, critterAttack);