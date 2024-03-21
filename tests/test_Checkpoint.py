import sst

# Define the simulation components
comp_c0 = sst.Component("c0", "coreTestElement.coreTestCheckpoint")
comp_c0.addParams({
      "starter" : 'T',
      "counter" : 1000
})

comp_c1 = sst.Component("c1", "coreTestElement.coreTestCheckpoint")
comp_c1.addParams({
      "starter" : 'F',
})

# Connect the components
link = sst.Link("link")
link.connect( (comp_c0, "port", "1us"), (comp_c1, "port", "1us") )
