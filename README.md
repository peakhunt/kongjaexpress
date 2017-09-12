# kongjaexpress

KongjaExpress is meant to be a puppy framework for server side development on Linux machines.
It is written in C and I never intend to add unnecessary unmanageable codes here.

It should be quite a good starting framework for service side deveopment either in single threaded or multi threaded environment.

Keep it simple!
Thread is evil! But sometimes it is necessary to fully take advantage of multi-core CPU.

Libev here is modified to support exceptfd. It is a simple change but for some reasons, original author refuses to implement it. But we need event on exceptfd on embedded linux environment to detect GPIO event.
Additionally I am planning to replace Libev with something more simpler in the future.
