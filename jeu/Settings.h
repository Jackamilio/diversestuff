#pragma once

#include "GuiReflection.h"

struct Settings
{
	float playerSpeed = 3.0f;
	int slashFrameDuration = 2;

	struct Enemy {
		float patrolSpeed = 1.0f;
		float patrolCenterDistance = 5.0f;
		float chaseSpeed = 1.5f;
		float sightDistance = 5.0f;
		
		struct Attack {
			float minInBetweenDuration = 5.0f;
			float maxInBetweenDuration = 10.0f;
			float windingDuration = 1.0f;
			float windingSpeed = -1.0f;
			float attackDuration = 1.0f;
			float attackSpeed = 2.0f;
			float postAttackPauseDuration = 0.5f;
		};
	};

	Enemy critter;
	Enemy::Attack critterAttack;
};

JSONANDGUI_REFLECTION(Settings::Enemy::Attack, minInBetweenDuration, maxInBetweenDuration, windingDuration, windingSpeed, attackDuration, attackSpeed, postAttackPauseDuration);
JSONANDGUI_REFLECTION(Settings::Enemy, patrolSpeed, patrolCenterDistance, chaseSpeed, sightDistance);
JSONANDGUI_REFLECTION(Settings, playerSpeed, slashFrameDuration, critter, critterAttack);