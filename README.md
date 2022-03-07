# DrumSampler

This is a very simple drum sampler plugin I made for my final thesis project in April 2019. 
Since when i made this there weren't so much online examples of VST development, especially drums/samplers, I decided to fix some stuff and upload the repository so anyone can learn from it. 

It consists of two synths (just Kick and Snare) with some of the most basic parameters for each Instrument channel (gain, pan, mute, solo, midi learn) and for the Master channel (level, pan, mute).
It's a VST2/VST3 plugin but it can be built as a standalone program too.

Important note: this software was made for educational purposes only, it has multiple bugs and frequent cracks/clips in the outcoming audio that I'm definitely not going to fix for now :) 
so if you want to use it as a working plugin, or even worse you're planning to start a similar project using this code, please don't! Refer to official JUCE tutorials instead.

GUI is deactivated so you have to run it with the AudioPluginHost, otherwise you will not see parameters!
Kick is pre-mapped to C4 and Snare to C#4.

Made with JUCE v6.0.5
