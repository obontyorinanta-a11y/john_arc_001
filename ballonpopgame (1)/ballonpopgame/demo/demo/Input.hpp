#ifndef INPUT_HPP
#define INPUT_HPP

// -----------------------------------------------------------------
//  Input.hpp - edge detection on top of the framework's key arrays.
//
//  This iGraphics version has NO iKeyboard() callback. It only keeps
//  keyPressed[] / specialKeyPressed[] filled while a key is held, and
//  calls fixedUpdate() every 16 ms. So a "press" must be detected as
//  the 0 -> 1 transition, otherwise one tap fires ~60 times a second.
//
//  Order of use inside fixedUpdate():
//        1. read keyTapped() / keyHeld() as much as you like
//        2. call inputEndFrame() ONCE at the very end
// -----------------------------------------------------------------

#include "Config.hpp"

const int KEY_ENTER  = 13;
const int KEY_ESCAPE = 27;
const int KEY_SPACE  = 32;

int prevKey[512]     = { 0 };
int prevSpecial[512] = { 0 };

// held down right now?
int keyHeld(unsigned char k)
{
	return isKeyPressed(k);
}

// pressed during this frame only (rising edge)
int keyTapped(unsigned char k)
{
	return (isKeyPressed(k) && !prevKey[k]);
}

// same, but accepts both letter cases: keyTappedCh('p','P')
int keyTappedCh(unsigned char lower, unsigned char upper)
{
	return (keyTapped(lower) || keyTapped(upper));
}

int specialHeld(int k)
{
	return isSpecialKeyPressed((unsigned char)k);
}

int specialTapped(int k)
{
	return (isSpecialKeyPressed((unsigned char)k) && !prevSpecial[k]);
}

// remember this frame's state so the next frame can compare against it
void inputEndFrame()
{
	int i;
	for (i = 0; i < 512; i++) {
		prevKey[i]     = isKeyPressed((unsigned char)i);
		prevSpecial[i] = isSpecialKeyPressed((unsigned char)i);
	}
}

#endif
