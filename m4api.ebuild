# Copyright 1999-2011 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=3

DESCRIPTION="M4-ATX power supply utility"
HOMEPAGE="http://ram.umd.edu/wiki/Public/Software/m4api"
SRC_URI="http://ram.umd.edu/git/bits/m4api.git/snapshot/refs/tags/${P}.tar.gz"

LICENSE="LGPL-2.1"
SLOT="0"
KEYWORDS="~x86 ~amd64"
IUSE=""

DEPEND="=dev-libs/libusb-0.1*"
RDEPEND="${DEPEND}"

S="${WORKDIR}"/m4api-${P}

src_compile() {
	emake || die 'emake'
}

src_install() {
	dobin m4ctl
}
