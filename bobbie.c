/* The "Bobbie" policer algorithm 
   I thought about filing a patent but called for another beer. */

/* Bobbie applies an adaptation of the codel algorithm 
   to manage a virtual clock for its drop schedule.

   This results in a kinder, gentler policer. Hopefully.

   When used with an ETX scheduler, packets are also pushed into the
   future. 

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Inbound policer filter.
   A list based version could skip the timestamping step.
   This a bit idealized assuming codel state variables elswhere */

// rate is in bytes/ns presently and a double because that makes my
// head hurt. Even trying to keep time as an int is a mistake.

/* Dave's Quick and Dirty packet emulator */

#define MAXP 1000 // max packets to test with
#define MAXSIM 10000 // run the sim for 10usec tops
#define ACT_SHOT (-1) // spoof ACT_SHOT

typedef double simtime;
typedef uint64_t u64;

struct sk_buff {
  int size;
  int hash;
  simtime timestamp;
};

struct codel_vars { // spoof codel for now
  double last_over;
};

u64 skb_set_time(struct sk_buff *skb, simtime t) {
  return skb->timestamp = t;
}

/* quick and dirty virtual clock */

static simtime now = 0;

simtime get_simtime() {
  return now;
}

simtime clock_advance(simtime ns) {
  now += ns;
  if(now > MAXSIM) exit(-1);
  return now;
}

simtime tick() { return clock_advance(1); }

struct shaperctl {
  double rate, optimum_rate, objective_rate;
  u64 bytes_over;
  simtime vclock;
};

struct filterctl {
  struct codel_vars cvars;
  struct shaperctl s;
} ;


// objective_rate = 10mbit/s;

u64 rate_set(struct shaperctl *s, u64 kbit, u64 interval) {
  // fancy math
  return 0;
}

/* Your classic boring rate estimator */

double update_rate(struct shaperctl *s, double rate, int size) {
  return 0;
}

/* The optimum rate is "special".
   Instead of slowing down the packets we speed up the clock,
   and fling the scheduling decision into the future. */

double optimum_rate(struct shaperctl *s, double rate, int size) {

  /* the bobbie part is we have to drain the overage */

  /* fling things less far into the future as we approach our goal */
  return 0;
  
}

// for testing drop every 4th packet we hit

struct sk_buff *should_drop(struct codel_vars *c, struct sk_buff *skb) {
  static int m = 0;
  if(++m % 4 == 0) return NULL;
  return skb;
}

int insert_skb(struct filterctl *f, struct sk_buff *skb) {
  u64 now;
  if(!(now = skb->timestamp)) 
    now = skb_set_time(skb, get_simtime());
  
  u64 rate = update_rate(&f->s, now, skb->size);
  u64 optimum = optimum_rate(&f->s, now, rate); // magic

  // fixme We need to turn this back into intervals
  
  if (rate > optimum) {
    // More magic
    f->s.vclock += now + (rate - optimum);
    skb_set_time(skb, f->s.vclock);
    if(should_drop(&f->cvars, skb)) {
      return ACT_SHOT;
    }
  } else {
    f->s.vclock = now;
  }
  return 0;
}

int main(int argc, char **argv) {
  double irate = atof(argv[1]);
  double orate = atof(argv[2]);
  struct sk_buff skb[MAXP];
  simtime s;
  struct filterctl f; // FIXME initialize this
  f.s = { .rate = 1, .optimum_rate = 1, .objective_rate = 1 };
  
  for(int i = 0; i < MAXP; i++) {
    skb[i].size = 1000;
    skb[i].hash = 0;
    skb[i].timestamp = i; // irate * clock++; // fixme 
    };
  
  tick();
  int i = 0;
  do {
    do {
      if(skb[i].timestamp <= now) {
	printf("%12g %s", now, insert_skb (&f,&skb[i]) == ACT_SHOT ?
	       "SHOT" : "SENT");
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


