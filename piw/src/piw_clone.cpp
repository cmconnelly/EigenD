
/*
 Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com

 This file is part of EigenD.

 EigenD is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 EigenD is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with EigenD.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <piw/piw_clone.h>
#include <piw/piw_clock.h>
#include <piw/piw_tsd.h>
#include <piw/piw_policy.h>
#include <piw/piw_fastdata.h>
#include <picross/pic_flipflop.h>

#include <map>

#define MAX_GATES 256

#define CLONER_DEBUG 0

namespace {
    struct clone_wire_t;

    struct clone_root_ctl_t: piw::root_ctl_t
    {
        clone_root_ctl_t(const piw::d2d_nb_t &f, unsigned filtered_signal) : filter_(f), filtered_signal_(filtered_signal) {};

        piw::d2d_nb_t filter_;
        unsigned filtered_signal_;
    };

    struct clone_wire_ctl_t: piw::wire_ctl_t, piw::event_data_source_real_t, piw::event_data_sink_t, piw::fastdata_t, virtual public pic::lckobject_t
    {
        clone_wire_ctl_t(unsigned o, piw::clone_t::impl_t *impl, const piw::d2d_nb_t &filter, unsigned filtered_signal, const piw::event_data_source_t &es);
        ~clone_wire_ctl_t();

        bool event_end(unsigned long long t);
        void event_buffer_reset(unsigned sig, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
        void source_ended(unsigned seq);
        void activate(bool b, unsigned long long t);

        bool fastdata_receive_event(const piw::data_nb_t &d,const piw::dataqueue_t &q);
        bool fastdata_receive_data(const piw::data_nb_t &d);
        void clear_buffer();
        void filter(unsigned long long t);
        void filter_data(unsigned long long t);
        void filter_id(unsigned long long t);

        bool is_gate_open();

        unsigned output_;
        piw::clone_t::impl_t *parent_;
        bool active_;
        pic::flipflop_functor_t<piw::d2d_nb_t> filter_;
        unsigned filtered_signal_;
        unsigned seq_;
        piw::dataholder_nb_t id_;
        piw::xevent_data_buffer_t *buffer_;
    };

    struct clone_wire_t: piw::wire_t
    {
        clone_wire_t(piw::clone_t::impl_t *p, const piw::event_data_source_t &es);
        ~clone_wire_t() { invalidate(); }

        void add_output(unsigned name, clone_root_ctl_t *root);
        void del_output(unsigned name);
        void del_outputs();
        int gc_clear();
        int gc_traverse(void *v, void *a) const;
        void wire_closed() { delete this; }
        void invalidate();
        void activate(bool b, unsigned o, unsigned long long t);

        piw::clone_t::impl_t *parent_;
        piw::event_data_source_t source_;
        pic::flipflop_t<pic::lckvector_t<clone_wire_ctl_t*>::nbtype > clones_;
    };

};

struct piw::clone_t::impl_t: root_t, virtual public pic::tracked_t
{
    impl_t(bool init): root_t(0), policy_(true) { memset(gate_,init?0xff:0x00,MAX_GATES/8); }
    ~impl_t() { invalidate(); }

    void invalidate();
    int gc_traverse(void *v, void *a) const;
    int gc_clear();
    piw::wire_t *root_wire(const event_data_source_t &es);
    void add_output(unsigned name, const piw::cookie_t cookie, const piw::d2d_nb_t &filter, unsigned filtered_signal);
    void del_outputs();
    void del_output(unsigned name);
    void root_clock();
    void root_latency();
    void gate(unsigned o, const piw::data_nb_t &d);
    void __gate(unsigned o, bool on, unsigned long long t);
    void setgate(unsigned o, bool b);
    bool getgate(unsigned o);

    void root_closed() { invalidate(); }
    void root_opened() { root_clock(); root_latency(); }

    pic::flipflop_t<pic::lckmap_t<piw::data_t,clone_wire_t *>::lcktype > children_;
    std::map<unsigned,clone_root_ctl_t*> clones_;
    unsigned long gate_[MAX_GATES/32];
    bool policy_;
};


/*
 * clone_wire_t
 */

clone_wire_t::clone_wire_t(piw::clone_t::impl_t *p, const piw::event_data_source_t &es): parent_(p), source_(es)
{
    parent_->children_.alternate().insert(std::make_pair(source_.path(),this));
    parent_->children_.exchange();
}

void clone_wire_t::invalidate()
{
    if(parent_)
    {
        disconnect();
        del_outputs();
        parent_->children_.alternate().erase(source_.path());
        parent_->children_.exchange();
        parent_=0;
    }
}

void clone_wire_t::activate(bool b, unsigned o, unsigned long long t)
{
    pic::flipflop_t<pic::lckvector_t<clone_wire_ctl_t *>::nbtype>::guard_t g(clones_);

    if(!(o<g.value().size()))
    {
        return;
    }

    clone_wire_ctl_t *c = g.value()[o];

    if(!c)
    {
        return;
    }

    c->activate(b,t);
}

void clone_wire_t::add_output(unsigned name, clone_root_ctl_t *root)
{
    if(name>clones_.alternate().size())
    {
        clones_.alternate().resize(name);
    }

    unsigned o = name-1;

    clone_wire_ctl_t *w = clones_.alternate()[o];

    if(!w)
    {
        w = new clone_wire_ctl_t(o,parent_, root->filter_, root->filtered_signal_, source_);
        clones_.alternate()[o] = w;
    }

    clones_.exchange();

    root->connect_wire(w,w->source());
}

void clone_wire_t::del_outputs()
{
    pic::lckvector_t<clone_wire_ctl_t *>::nbtype::iterator oi;

    for(oi=clones_.alternate().begin(); oi!=clones_.alternate().end(); oi++)
    {
        clone_wire_ctl_t *w = *oi;
        if(w) delete w;
    }

    clones_.alternate().resize(0);
    clones_.exchange();
}

void clone_wire_t::del_output(unsigned name)
{
    unsigned o = name-1;

    if(o<clones_.alternate().size())
    {
        if(clones_.alternate()[o])
        {
            delete clones_.alternate()[o];
            clones_.alternate()[o] = 0;
        }
    }

    clones_.exchange();
}

int clone_wire_t::gc_traverse(void *v, void *a) const
{
    pic::lckvector_t<clone_wire_ctl_t *>::nbtype::const_iterator oi;
    int r;

    for(oi=clones_.current().begin(); oi!=clones_.current().end(); oi++)
    {
        clone_wire_ctl_t *w = *oi;
        if(w && (r=w->filter_.gc_traverse(v,a))!=0)
        {
            return r;
        }
    }

    return 0;
}

int clone_wire_t::gc_clear()
{
    pic::lckvector_t<clone_wire_ctl_t *>::nbtype::const_iterator oi;

    for(oi=clones_.alternate().begin(); oi!=clones_.alternate().end(); oi++)
    {
        clone_wire_ctl_t *w = *oi;
        if(w) w->filter_.gc_clear();
    }

    clones_.exchange();

    return 0;
}


/*
 * clone_wire_ctl_t
 */

clone_wire_ctl_t::clone_wire_ctl_t(unsigned o, piw::clone_t::impl_t *impl, const piw::d2d_nb_t &filter, unsigned filtered_signal, const piw::event_data_source_t &es):
    piw::event_data_source_real_t(es.path()), piw::fastdata_t(PLG_FASTDATA_SENDER), output_(o), parent_(impl), active_(false), filter_(filter), filtered_signal_(filtered_signal), buffer_(0)
{
    subscribe_and_ping(es);

    piw::tsd_fastdata(this);
    enable(true,false,false);
}

clone_wire_ctl_t::~clone_wire_ctl_t()
{
    unsubscribe();
    disconnect();
    end_slow(piw::tsd_time());
    source_shutdown();
    clear_buffer();
}

void clone_wire_ctl_t::event_buffer_reset(unsigned sig, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    if(active_)
    {
        source_buffer_reset(sig,t,o,n);
    }
}

void clone_wire_ctl_t::clear_buffer()
{
    if(buffer_ != 0)
    {
        buffer_->clear();
        delete buffer_;
        buffer_ = 0;
    }
}

void clone_wire_ctl_t::filter(unsigned long long t)
{
    if(filtered_signal_)
    {
        filter_data(t);
    }
    else
    {
        filter_id(t);
    }
}

void clone_wire_ctl_t::filter_data(unsigned long long t)
{
    piw::data_nb_t d;
    if(current_data().latest(filtered_signal_,d,t))
    {
        piw::data_nb_t nd = filter_(d);

        bool valid = !nd.is_null();
#if CLONER_DEBUG>0
        pic::logmsg() << "filtered cloner " << (void *)this << " on " << output_ << " filtering data of event " << nd << "<-" << d << " : valid=" << valid;
#endif
        if(valid)
        {
            PIC_ASSERT(buffer_ == 0);
            buffer_ = new piw::xevent_data_buffer_t(SIG1(filtered_signal_),PIW_DATAQUEUE_SIZE_ISO);

            active_ = true;
            id_.set_nb(current_id());
            buffer_->merge(current_data(), 0);
            send_fast(current_id(),current_data().signal(filtered_signal_));
            source_start(seq_,current_id().restamp(t),*buffer_);
        }
    }
}

void clone_wire_ctl_t::filter_id(unsigned long long t)
{
    piw::data_nb_t nid = filter_(current_id());

    bool valid = nid.is_path();
#if CLONER_DEBUG>0
    pic::logmsg() << "cloner " << (void *)this << " on " << output_ << " filtering id of event " << nid << "<-" << current_id() << " : valid=" << valid;
#endif
    if(valid)
    {
        active_ = true;
        id_.set_nb(nid);
        source_start(seq_,nid.restamp(t),current_data());
    }
}

bool clone_wire_ctl_t::is_gate_open()
{
    bool result = parent_->getgate(output_);
#if CLONER_DEBUG>0
    pic::logmsg() << "cloner " << (void *)this << " on " << output_ << " is_gate_open output=" << output_ << " result=" << result;
#endif
    return result;
}

void clone_wire_ctl_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
#if CLONER_DEBUG>0
    pic::logmsg() << "cloner " << (void *)this << " on " << output_ << " event_start seq=" << seq << " id=" << id;
#endif
    seq_ = seq;

    if(is_gate_open())
    {
        filter(id.time());
    }
}

bool clone_wire_ctl_t::event_end(unsigned long long t)
{
    seq_ = 0;

    if(active_)
    {
        active_ = false;
        bool v = source_end(t);
        clear_buffer();
#if CLONER_DEBUG>0
        pic::logmsg() << "cloner " << (void *)this << " on " << output_ << " ending event " << id_ << ": " << v;
#endif
        return v;
    }

    return true;
}

bool clone_wire_ctl_t::fastdata_receive_event(const piw::data_nb_t &d, const piw::dataqueue_t &q)
{
#if CLONER_DEBUG>0
    pic::logmsg() << "filtered cloner (re) " << (void *)this << " on " << output_ << " receiving event " << d;
#endif
    ping(d.time(),q);
    return true;
}

bool clone_wire_ctl_t::fastdata_receive_data(const piw::data_nb_t &d)
{
    if(buffer_)
    {
        piw::data_nb_t nd = filter_(d);
#if CLONER_DEBUG>0
        pic::logmsg() << "filtered cloner (rd) " << (void *)this << " on " << output_ << " filtering data of event " << nd << "<-" << d;
#endif
        buffer_->add_value(filtered_signal_, nd);

        return true;
    }
#if CLONER_DEBUG>0
        pic::logmsg() << "filtered cloner (rd) " << (void *)this << " on " << output_ << " filtering data of event not valid";
#endif

    return false;
}

void clone_wire_ctl_t::activate(bool b, unsigned long long t)
{
#if CLONER_DEBUG>0
    pic::logmsg() << "cloner " << (void *)this << " on " << output_ << " activation " << b << " current: " << current_id();
#endif

    if(!current_id().is_path())
    {
        return;
    }

    if(b)
    {
        if(parent_->policy_)
        {
#if CLONER_DEBUG>0
            pic::logmsg() << "cloner " << (void *)this << " on " << output_ << " activating event " << current_id();
#endif
            filter(t);
        }
    }
    else
    {
        active_ = false;
#if CLONER_DEBUG>0
        pic::logmsg() << "cloner " << (void *)this << " on " << output_ << " deactivating event " << current_id();
#endif
        source_end(t);
        clear_buffer();
    }
}

void clone_wire_ctl_t::source_ended(unsigned seq)
{
    if(!is_gate_open())
    {
        event_ended(seq);
    }
}


/*
 * clone_t::impl_t
 */

void piw::clone_t::impl_t::invalidate()
{
    tracked_invalidate();

    pic::lckmap_t<piw::data_t,clone_wire_t *>::lcktype::iterator ci;

    while((ci=children_.alternate().begin())!=children_.alternate().end())
    {
        delete ci->second;
    }
}

int piw::clone_t::impl_t::gc_traverse(void *v, void *a) const
{
    pic::lckmap_t<piw::data_t,clone_wire_t *>::lcktype::const_iterator ci;
    int r;

    for(ci=children_.current().begin(); ci!=children_.current().end(); ci++)
    {
        if((r=ci->second->gc_traverse(v,a))!=0)
        {
            return r;
        }
    }

    return 0;
}

int piw::clone_t::impl_t::gc_clear()
{
     pic::lckmap_t<piw::data_t,clone_wire_t *>::lcktype::iterator ci;

    for(ci=children_.alternate().begin(); ci!=children_.alternate().end(); ci++)
    {
        ci->second->gc_clear();
    }

    children_.exchange();

    return 0;
}

piw::wire_t *piw::clone_t::impl_t::root_wire(const event_data_source_t &es)
{
    pic::lckmap_t<piw::data_t,clone_wire_t *>::lcktype::iterator ci;

    if((ci=children_.alternate().find(es.path()))!=children_.alternate().end())
    {
        delete ci->second;
    }

    clone_wire_t *w = new clone_wire_t(this,es);

    std::map<unsigned,clone_root_ctl_t *>::iterator oi;
    for(oi=clones_.begin(); oi!=clones_.end(); oi++)
    {
        w->add_output(oi->first,oi->second);
    }

    return w;
}

void piw::clone_t::impl_t::add_output(unsigned name, const piw::cookie_t cookie, const piw::d2d_nb_t &filter, unsigned filtered_signal)
{
    del_output(name);

    clone_root_ctl_t *r;

    PIC_ASSERT(clones_.find(name)==clones_.end());

    r = new clone_root_ctl_t(filter, filtered_signal);
    clones_.insert(std::make_pair(name,r));
    r->set_clock(get_clock());
    r->set_latency(get_latency());
    r->connect(cookie);

    pic::lckmap_t<piw::data_t,clone_wire_t *>::lcktype::iterator ci;

    for(ci=children_.alternate().begin(); ci!=children_.alternate().end(); ci++)
    {
        ci->second->add_output(name, r);
    }

    children_.exchange();
}

void piw::clone_t::impl_t::del_outputs()
{
    std::map<unsigned,clone_root_ctl_t *>::iterator oi;

    while((oi=clones_.begin())!=clones_.end())
    {
        delete oi->second;
        clones_.erase(oi);
    }

    pic::lckmap_t<piw::data_t,clone_wire_t *>::lcktype::iterator ci;

    for(ci=children_.alternate().begin(); ci!=children_.alternate().end(); ci++)
    {
        ci->second->del_outputs();
    }

    children_.exchange();
}

void piw::clone_t::impl_t::del_output(unsigned name)
{
    std::map<unsigned,clone_root_ctl_t *>::iterator oi;

    if((oi=clones_.find(name))!=clones_.end())
    {
        delete oi->second;
        clones_.erase(oi);
    }

    pic::lckmap_t<piw::data_t,clone_wire_t *>::lcktype::iterator ci;

    for(ci=children_.alternate().begin(); ci!=children_.alternate().end(); ci++)
    {
        ci->second->del_output(name);
    }

    children_.exchange();
}

void piw::clone_t::impl_t::root_clock()
{
    std::map<unsigned,clone_root_ctl_t *>::iterator oi;

    for(oi=clones_.begin(); oi!=clones_.end(); oi++)
    {
        oi->second->set_clock(get_clock());
    }
}

void piw::clone_t::impl_t::root_latency()
{
    std::map<unsigned,clone_root_ctl_t *>::iterator oi;

    for(oi=clones_.begin(); oi!=clones_.end(); oi++)
    {
        oi->second->set_latency(get_latency());
    }
}

void piw::clone_t::impl_t::gate(unsigned o, const piw::data_nb_t &d)
{
    if(d.as_arraylen() == 0) return;

    bool on = (d.as_norm()!=0.0);
    __gate(o,on,d.time());
}

void piw::clone_t::impl_t::__gate(unsigned o, bool on, unsigned long long t)
{
#if CLONER_DEBUG>0
    pic::logmsg() << "cloner __gate output=" << o << " on=" << on << " current=" << getgate(o);
#endif
    if(on == getgate(o))
    {
        return;
    }

    setgate(o,on);

    pic::flipflop_t<pic::lckmap_t<piw::data_t,clone_wire_t *>::lcktype >::guard_t wg(children_);
    pic::lckmap_t<piw::data_t,clone_wire_t *>::lcktype::const_iterator wi;

    for(wi=wg.value().begin(); wi!=wg.value().end(); wi++)
    {
        wi->second->activate(on,o,t);
    }

}

void piw::clone_t::impl_t::setgate(unsigned o, bool b)
{
    unsigned index = o/32;
    unsigned mask = 1<<(o%32);
    if(b)
    {
        gate_[index]|=mask;
    }
    else
    {
        gate_[index]&=(~mask);
    }
}

bool piw::clone_t::impl_t::getgate(unsigned o)
{
    unsigned index = o/32;
    unsigned mask = 1<<(o%32);
    return (gate_[index]&mask)!=0;
}


/*
 * clone_t
 */

namespace
{
    struct gatesink_t: pic::sink_t<void(const piw::data_nb_t &)>
    {
        gatesink_t(piw::clone_t::impl_t *i, unsigned o) : impl_(i), output_(o)
        {
        }

        void invoke(const piw::data_nb_t &d) const
        {
            impl_->gate(output_,d);
        }

        bool iscallable() const
        {
            return impl_.isvalid();
        }

        bool compare(const pic::sink_t<void(const piw::data_nb_t &)> *s_) const
        {
            const gatesink_t *s = dynamic_cast<const gatesink_t *>(s_);
            return (s && s->impl_==impl_ && s->output_==output_);
        }

        pic::weak_t<piw::clone_t::impl_t> impl_;
        unsigned output_;
    };
}

piw::cookie_t piw::clone_t::cookie()
{
    return piw::cookie_t(root_);
}

piw::clone_t::clone_t(bool init): root_(new impl_t(init))
{
}

piw::clone_t::~clone_t()
{
    delete root_;
}

void piw::clone_t::set_filtered_output(unsigned name, const piw::cookie_t &cookie, const piw::d2d_nb_t &filter)
{
    root_->add_output(name, cookie, filter, 0);
}

void piw::clone_t::set_filtered_data_output(unsigned name, const piw::cookie_t &cookie, const piw::d2d_nb_t &filter, unsigned filtered_signal)
{
    PIC_ASSERT(filtered_signal>0);
    root_->add_output(name, cookie, filter, filtered_signal);
}

void piw::clone_t::set_output(unsigned name, const piw::cookie_t &cookie)
{
    root_->add_output(name, cookie, piw::null_filter(), 0);
}

piw::change_nb_t piw::clone_t::gate(unsigned name)
{
    PIC_ASSERT(name>0);

    unsigned o = name-1;
    PIC_ASSERT(o<MAX_GATES);

    return piw::change_nb_t(pic::ref(new gatesink_t(root_,o)));
}

void piw::clone_t::clear_output(unsigned name)
{
    root_->del_output(name);
}

int piw::clone_t::gc_traverse(void *v, void *a) const
{
    return root_->gc_traverse(v,a);
}

int piw::clone_t::gc_clear()
{
    return root_->gc_clear();
}

static int __enable(void *r_, void *o_, void *e_)
{
    piw::clone_t::impl_t *r = (piw::clone_t::impl_t *)r_;
    unsigned o = *(unsigned *)o_;
    bool e = *(bool *)e_;
    r->__gate(o,e,piw::tsd_time());
    return 0;
}

void piw::clone_t::set_policy(bool policy)
{
    root_->policy_ = policy;
}

void piw::clone_t::enable(unsigned name,bool enabled)
{
    PIC_ASSERT(name>0);
    unsigned o = name-1;
    piw::tsd_fastcall3(__enable,root_,&o,&enabled);
}
