# About "bobbie"

Inbound shaping has worked well for us, except when we run out of cpu,
which is often enough to be bothersome on low end hardware at high rates.

Early versions of cake used 40% less cpu than the equivalent tbf + fq_codel
combination. The final version uses twice that at high rates.

There remains a need to do inbound *something* on cheap hardware. Perhaps
revisiting the notion of policing using modern aqm techniques would work.

## Other flaws of inbound shaping

* We inflict at least 1/6 BDP presently (eventually) by using fq-codel
  we're deleriously happy about fq_codel but it does create greater RTT
  for fat flows

* We inflict some delay even on sparse flows

# The open questions

* Can a better policer be built today?
  
* Is it possible to store all the data in the pipe? While achieving
some level of flow fairness and at the same time provide reasonably
good throughput?

# Bobbie's name

Bobbie got its name when I thought deeply about how codel actually
worked, that it aimed for a target delay and then stopped. Aiming
with a different goal (a target rate), while still applying the gradual
increase in drop policy that the rest of codel does to find and hold
the "right drop rate", or "to make sure the upstream token bucket does not
drain completely", so it would "bob" up and down seemed like a better 
idea than a classic tri-color or brick wall "policer".  The fact that a 
policeman, armed only with a stick, is called a "bobbie" in England,
was icing on the cake.

But, after cake turned in such nice early results, and lacking time,
team, and money, I abandoned the idea. I *like* that a little
buffering and fq get things into flow balance rapidly, and the
per host FQ in cake is the bomb.

But I hate running out of cpu to do it.

# PLAN

* Initial target bandwidth: 100Mbit and above
* cpu usage: 1/2 tbf+fq_codel: 120mbits on mips class hw
* stay within +- 10% of the targetted bandwidth
* while absorbing TB-size bursts
* use ecn also but shoot as overage approaches maximum

# Ideas

Policer "classic", tri color, compared over varous workloads -
	as near as I can tell the policers in linux are essentially broken
PI-ish based one - again with a target rate not latency
4 phase one (kind of like BBR)
codel based
cake-ish - leveraging the proportional fairness idea there

There's only a little research on this subject, with a reasonably
narrow patent by... guess who - nichols and jacobson, in 2000.

There was a lot of work done later, without any usable code
by the ConEx effort and others.

As I'm mostly deeply familiar with codel...

I figure on just coding up something like that. Maybe
the invsqrt method against time is not right, maybe something
harder, based on the upstream based TB tokens remaining is better.

I've always worried about the "decay" phase of codel, also.

# LONG TERM PLAN

If it works out produce an epbf version, and try to look into an fq-ish version

# Random thoughts
## "TART"

Given our preference for food puns and the really nice backronym
"Transparent automatic rate translator" I was really tempted to
call this that rather than bobbie, but I'm painfully aware of the
non-PC meaning.

## Time per byte breaks down at 10gigE with nanosec

a single 64byte packet is 6ns. 

Picosec seems required or using a shifted value, to get something right

