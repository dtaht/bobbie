/* The "Bobbie" policer algorithm 
   I thought about filing a patent. */

/* Bobbie applies an adaptation of the codel algorithm 
   to manage a virtual clock for its drop schedule.

   This results in a kinder, gentler policer. Hopefully.

   When used with an ETX scheduler, packets are also pushed into the
   future. 

*/

#include <stdio.h>
#include <stdint.h>

/* Inbound policer filter.
   A list based version could skip the timestamping step.
   This a bit idealized assuming codel state variables elswhere */

// rate is in bytes/ns presently and a double because that makes my
// head hurt. Even trying to keep time as an int is a mistake.

/* Dave's Quick and Dirty packet emulator */

#define ACT_SHOT = (-1)
typedef simtime double;

struct skbuff {
  int size;
  int hash;
  simtime timestamp;
};

u64 skb_set_time(struct skbuff *skb, u64 t) {
  return skb->timestamp = t;
}

/* quick and dirty virtual clock */

static simtime now = 0;

simtime get_simtime() {
  return now;
}

#define MAXSIM 64000

simtime clock_advance(ns) {
  now += ns;
  if(now > MAXSIM) exit(-1);
  return now;
}

simtime tick() { return clock_advance(1); }

struct shapectl {
  double rate,optimum_rate, objective_rate;
  u64 bytes_over;
  simtime vclock;
};

struct filterctl {
  struct codel_vars cvars;
  struct shapectl s;
} ;


// objective_rate = 10mbits;

rate_set(struct shapectl s, u64 kbit, u64 interval) {
  // fancy math
}

/* Your classic boring rate estimator */

double update_rate(struct shaperctl *s, double rate, size) {
}

/* The optimum rate is "special".
   Instead of slowing down the packets we speed up the clock,
   and fling the scheduling decision into the future. */

double optimum_rate(struct shaperctl *s, double rate, size) {

  /* the bobbie part is we have to drain the overage */

  /* fling things less far into the future as we approach our goal */
  
}

// for testing drop every 4th packet we hit

skb should_drop(f->cvars, skb) {
  static m = 0;
  if(++m % 4 == 0) return NULL;
  return skb;
}

int insert_skb(struct filterctl *f, skb_buff *skb) {
    u64 now;
    if(!now = skb->timestamp) 
          now = timestamp(skb);
    rate = update_rate(f->s, now, skb->size);
    optimum_rate = optimium_rate(f, now, rate); // magic
    if (rate > optimimum) {
      // More magic
      f->s.vclock += now + (rate - optimum_rate);
      skb_set_time(skb, f->s.vclock);
      if(should_drop(f->cvars, skb)) {
	return ACT_SHOT;
      }
    } else {
	f->s.vclock = now;
    }
    return 0;
}

main(int argv, char **argv) {
  double irate = atod(argv[1]);
  double orate = atod(argv[2]);
  struct skb_buff skb[MAXP];
  simtime s;
  
  for(int i = 0; i < MAXP; i++)
    skb[i]. {
      size = 1000;
      hash = 0;
      timestamp = irate * clock++; // fixme 
    };
  
  tick();
  int i = 0;
  do {
    do {
      if(pkt[i]->timestamp <= now) {
	printf("%12g %s", now, insert(skb) == ACT_SHOT : "SHOT", "SENT");
	i++;
      }
    } while(now = tick());
  } while (i<MAXP);
}

/*

After transiting the system....

this arrives at the etx-codel qdisc, which just pulls from the timer wheel
as fast as possible, but no faster. Like codel it makes drop decisions based
on time in queue, but the exception is that that queue is now over in
the timer wheel, we can't pull from it until we get there - but we are
inserting into it at the speed of the inbound link, so we consider
stuff with the same departure time in order of the time it actually arrived.

Theoretically this makes fq-ing the above possible. But I'd rather
focus on the aqm bit right now.

*/


