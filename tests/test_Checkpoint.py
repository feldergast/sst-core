import sst

sst.setProgramOption("checkpoint-period", "500us")

# Define the simulation components
comp_c0 = sst.Component("c0", "coreTestElement.coreTestCheckpoint")
comp_c0.addParams({
      "starter" : 'T',
      "counter" : 1000,
      "clock_frequency" : "100 kHz",
      "clock_duty_cycle" : 20,
      "test_string" : "hi"
})

comp_c1 = sst.Component("c1", "coreTestElement.coreTestCheckpoint")
comp_c1.addParams({
      "starter" : 'F',
      "clock_frequency" : "100 kHz",
      "clock_duty_cycle" : 15,
      "test_string" : "hello",
      "outputprefix" : "c1 talking",
      "outputverbose" : 2,
})

# Connect the components
link = sst.Link("link")
link.connect( (comp_c0, "port", "1us"), (comp_c1, "port", "1us") )
