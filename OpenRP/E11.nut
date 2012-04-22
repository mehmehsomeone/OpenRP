function LoadE11Effects()
{
	E11.SetMuzzleEffect(RegisterEffect("blaster/muzzle_flash"));
	E11.SetSelectSound(RegisterSound("sound/weapons/blaster/select.wav"));
	E11.SetFlashSound(RegisterSound("sound/weapons/blaster/fire.wav"));
	E11.SetFiringSound(NULL_SOUND);
}
