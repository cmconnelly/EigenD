
#
# Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com
#
# This file is part of EigenD.
#
# EigenD is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# EigenD is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with EigenD.  If not, see <http://www.gnu.org/licenses/>.
#

import piw
from pi import agent,domain,bundles,atom
from plg_osc import osc_output_version as version

#
# osc_plg_native is generated by the build system
#

from osc_plg_native import osc_server

#
# Please read the Roadmap in plg_osc for soem background information.
#

#
# The osc output agent
#

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        #
        # Agents are constructed with the address and ordinal.  Each
        # agent will have a unique address and an ordinal unique for
        # that type of agent.
        #
        # We also pass in the automatically generated version to go into
        # the metadata.
        #
        agent.Agent.__init__(self, signature=version,names='osc output',ordinal=ordinal)

        #
        # Create a clock domain.  A clock domain is a collection of clock
        # sinks all of which share common properties.  Domains can be isochronous
        # or anisochronous.  Currently, sinks in iso domains can be connected
        # only to other iso domains using the same source.
        #
        self.domain = piw.clockdomain_ctl()

        #
        # Make our domain anisochronous (aniso is used for everything except audio)
        #
        self.domain.set_source(piw.makestring('*',0))

        #
        # Create an osc server, implemented in our C++ library
        #
        self.osc = osc_server(self.domain, "keyboard_%d" % ordinal)

        #
        # Create an osc output within the OSC server, which will correlate 4
        # streams.  These will be used for keyboard data.  We group like this
        # so that we can send all the keyboard parameters together.
        # these will be under the /key OSC path
        #
        # The return value from create_output is a piw.cookie, which represents
        # a connection endpoint.  This can be used by another component to send
        # events for OSC output.  The OSC output will place the first 4 signals
        # into each OSC output packet.
        #
        self.key_output = self.osc.create_output("key",True,4)

        #
        # Create an input.  An input does the work of correlating the events on a 
        # number of signals, grouping them together.
        # 
        # A Vector input is a proper multiple channel input.  This one is being
        # constructed to handle 4 signals, numbered 1,2,3 and 4  The events will
        # be sent to the OSC output stage.
        #
        self.key_input = bundles.VectorInput(self.key_output, self.domain,signals=(1,2,3,4))

        # Create a null atom just to group our inputs 
        
        self[1] = atom.Atom()

        #
        # Now create our inputs.  The data policy for the atom is created by our vector input.
        # There are lots of corner cases when we invert our model from signal-contains-events
        # to event-contains-signals.  In particular, what happens when events on individual
        # signals dont start and and stop together.
        #
        # Any input with a vector_policy will create or join an existing event.
        # merge_policy will never create events, just join onto existing events created by
        # a vector policy.
        #
        # We create 4 inputs, with vector policies routing them to signals 1,2,3 and 4 on
        # our bundle.  These are all non iso inputs.
        #

        self[1][1] = atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.key_input.vector_policy(1,False), names='activation input')
        self[1][2] = atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.key_input.vector_policy(2,False), names='pressure input')
        self[1][3] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.key_input.vector_policy(3,False), names='roll input')
        self[1][4] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.key_input.vector_policy(4,False), names='yaw input')

        #
        # Create another OSC output for the breath pipe.
        #

        self.breath_output = self.osc.create_output("breath",False,1)

        #
        # And another input.  We use separate inputs for breath and keys, because we want
        # all the key signals to be correlated together (so we can output pressure roll and
        # yaw as 1 packet)  but the breath pipe is completely independent.
        #
        self.breath_input = bundles.VectorInput(self.breath_output, self.domain,signals=(1,))
        self[1][5] = atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.breath_input.vector_policy(1,False),names='breath input')

        #
        # Lastly, create an atom called 'decimation' to store our current decimation rate.
        # This will appear in stage and can be set using belcanto.  When it changes, our
        # __decimation method will be called with a python numeric argument (because its a 
        # numeric domain)
        #

        self[3]=atom.Atom(domain=domain.BoundedInt(0,100),policy=atom.default_policy(self.__decimation),names='decimation')

    def __decimation(self,value):
        self[3].set_value(value)


#
# Define Agent as this agents top level class
#

agent.main(Agent)
