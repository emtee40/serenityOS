/*
 * Copyright (c) 2022, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Bindings/Intrinsics.h>
#include <LibWeb/HTML/PageTransitionEvent.h>

namespace Web::HTML {

PageTransitionEvent* PageTransitionEvent::create(JS::Realm& realm, DeprecatedFlyString const& event_name, PageTransitionEventInit const& event_init)
{
    return realm.heap().allocate<PageTransitionEvent>(realm, realm, event_name, event_init);
}

PageTransitionEvent* PageTransitionEvent::construct_impl(JS::Realm& realm, DeprecatedFlyString const& event_name, PageTransitionEventInit const& event_init)
{
    return create(realm, event_name, event_init);
}

PageTransitionEvent::PageTransitionEvent(JS::Realm& realm, DeprecatedFlyString const& event_name, PageTransitionEventInit const& event_init)
    : DOM::Event(realm, event_name, event_init)
    , m_persisted(event_init.persisted)
{
}

PageTransitionEvent::~PageTransitionEvent() = default;

void PageTransitionEvent::initialize(JS::Realm& realm)
{
    Base::initialize(realm);
    set_prototype(&Bindings::ensure_web_prototype<Bindings::PageTransitionEventPrototype>(realm, "PageTransitionEvent"));
}

}
