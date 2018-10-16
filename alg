
rate = data/interval;

In our equation, rate and interval are constants.

rate = 100.
interval = 10ms.

so what we want to do is solve for data

rate * interval = interval * data/interval or just data.

actual_rate * interval = data.

ideal_rate * interval = data.

But the real transform we want is to move clock forward so

bad_interval = data/actual_rate;

good_interval = data/ideal_rate

so if we got more data than we should handle in 10ms, and it should have been
20ms, that's what we add to the virtual clock. The virtual clock moves
ahead faster (monotonically) than the real clock and only when 

All this would be ok



