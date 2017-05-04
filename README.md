# UnpackAndAssemble
Unpack and Assemble

Idea is to implement the algorithms detailed in "OmniUnpack: Fast, Generic, and Safe Unpacking of Malware" paper.


CreateProcess: Capable of hooking process creation.
http://www.rohitab.com/discuss/topic/40560-get-process-name-form-pid-in-kernel-mode-driver/ : Create Process Callback

hookingIDT:    Hooking IDT, specifically 0x2E which is triggered in case of syscalls
https://github.com/proteansec/visual-studio-projects/tree/83953e5b6979e1b4a4687b703361cf3d02ca4176/hookidt

hookingSSDT:   Hooking SSDT, specifically "ZwWriteFile"
https://github.com/proteansec/visual-studio-projects/tree/83953e5b6979e1b4a4687b703361cf3d02ca4176/hookssdt

ollybone:      Omniunpack is inspired from this project, detects page execution for specified pages.
https://github.com/JohnTroony/Plugme-OllyDBGv1.0/tree/master/OllyBone%20v0.1

shadow:        Ollybone is inspired from this project. This hides desired kernel drivers by monitoring execution and writing operations in pages belongs to specified driver.
https://github.com/bowlofstew/rootkit.com/tree/master/hoglund/Shadow%20Walker%201.0

unpackerdriver: We tried to implement OmniUnpack under this project. Time limitation...
User-level-expeirements: We put our 3 weeks user level experiments under this folder.
