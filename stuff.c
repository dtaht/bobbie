
f->over_start > 
delta = now - f->last;
ideal = delta * f->byte_per_time;
actual = delta - skb->len * time_per_byte;;

if(f->over_start * 4ms > now) (re) start the drop schedule;

// At 100mbit, this is about 30 packets in flight. It's also
// over the average interval for wifi and cable, so used for
// smoothing things out.

// it's also something we could use to do out of band rate
// estimation much like pie

overage += actual;

#ifdef 0
// I long for the day we measure entrance and egress to thw whole system

if (!skb->tstamp) {
	skb->tstamp = now;
}
// I'd like to measure how much we get an ingress hash now

if (!skb->hash) {
}
#endif

bobbie_should_drop();


